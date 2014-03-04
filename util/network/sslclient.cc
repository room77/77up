//
// Copyright 2007 OpTrip, Inc.
//
// Author: Calvin Yang
//

#include <mutex>
#include <netdb.h>

// openssl headers
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "util/network/sslclient.h"
#include "util/network/dnslookup.h"
#include "util/string/strutil.h"

// singleton class that keep track of array of mutexes for SSL to use
// in a multi-threaded environment
class SSLMutexArray {
 protected:
  SSLMutexArray() {
    VLOG(3) << "Initializing SSL for multi-threaded environment...";
    num_locks_ = CRYPTO_num_locks();
    mutex_array_ = new mutex[num_locks_];
  }
 public:
  ~SSLMutexArray() {
    delete [] mutex_array_;
  }
  static SSLMutexArray& Instance() {
    static SSLMutexArray the_one;
    return the_one;
  }
  inline void Lock(int n) {
    ASSERT(n >= 0 && n < num_locks_);
    mutex_array_[n].lock();
  }
  inline void Unlock(int n) {
    ASSERT(n >= 0 && n < num_locks_);
    mutex_array_[n].unlock();
  }
 private:
  int num_locks_;
  mutex* mutex_array_;
};

// singleton class that does SSL initialization once
class SSLInit {
 public:
  ~SSLInit() {
    CRYPTO_set_locking_callback(NULL);
  };
  static const SSLInit& Instance() {
    static SSLInit the_one;
    return the_one;
  }
 protected:
  SSLInit() {
    VLOG(3) << "Initializing SSL...";
    // ssl initializations
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    SSL_library_init();
    //  OpenSSL_add_all_algorithms();

    // initialize for multi-threaded environment
    SSLMutexArray::Instance();  // force initialization here
    CRYPTO_set_locking_callback(pthreads_locking_callback);
    CRYPTO_set_id_callback(pthreads_id_callback);
  }

  static void pthreads_locking_callback(int mode, int n,
                                        const char *file, int line) {
    SSLMutexArray& mutex_array = SSLMutexArray::Instance();
    if (mode & CRYPTO_LOCK)
      mutex_array.Lock(n);
    else
      mutex_array.Unlock(n);
  }

  static unsigned long pthreads_id_callback() {
    return static_cast<unsigned long>(pthread_self());
  }
};

SSLClient::SSLClient() {
  SSLInit::Instance();  // init only when this is first called

  bio_ = NULL;
  ctx_ = SSL_CTX_new(SSLv23_client_method());
  if (!ctx_) {
    LOG(INFO) << "Error: " << ERR_reason_error_string(ERR_get_error());
    ShutdownSystem(23);
  }
  ssl_ = NULL;
  socket_ = -1;
}

SSLClient::~SSLClient() {
  CloseConnection();

  if (ctx_) {
    SSL_CTX_free(ctx_);
    ctx_ = NULL;
  }

  // bio_ and ssl_ should already be NULL (cleared by CloseConnection())
  ASSERT(bio_ == NULL);
  ASSERT(ssl_ == NULL);
}


// establish connection to server
// (if called again with empty hostname, re-establish connection to the
//  same server)
bool SSLClient::EstablishConnection(const string& hostname, int portnum) {
  clear_reply();

  if (!hostname.empty() && portnum >= 0) {
    hostname_ = hostname;
    portnum_ = portnum;
  }

  ASSERT(!hostname_.empty());
  ASSERT_GE(portnum_, 0);

  //  return EstablishInsecureConnection();
  return EstablishSecureConnection();
}


bool SSLClient::EstablishInsecureConnection() {
  stringstream server(stringstream::out);
  server << hostname_ << ":" << portnum_;
  const string& server_str = server.str();

  VLOG(3) << "connecting to " << server_str << "...";

  char *s =
    strutil::CopyStringLengthN(server_str.c_str(), server_str.size());
  socket_ = -1;
  bio_ = BIO_new_connect(s);
  delete [] s;

  if (!bio_) {
    LOG(INFO) << "cannot connect to " << server.str();
    return false;
  }
  if(BIO_do_connect(bio_) <= 0) {
    // connection failed
    LOG(INFO) << "error connecting to " << server.str();
    return false;
  }
  socket_ = BIO_get_fd(bio_, NULL);
  return true;
}


bool SSLClient::EstablishSecureConnection() {
  stringstream server(stringstream::out);
  server << hostname_ << ":" << portnum_;
  const string& server_str = server.str();

  VLOG(3) << "connecting to " << server_str << "...";

  // set up BIO object for secure connection
  if (bio_) {
    VLOG(3) << "Closing existing connection before establishing a new one";
    CloseConnection();
  }
  socket_ = -1;
  bio_ = BIO_new_ssl_connect(ctx_);
  BIO_get_ssl(bio_, &ssl_);
  SSL_set_mode(ssl_, SSL_MODE_AUTO_RETRY);

  // open secure connection

  string addr;
  if (!DNSUtil::Instance().LookupHost(hostname_, portnum_, &addr)) {
    LOG(INFO) << "failed to resolve connecting to " << server.str();
    ERR_print_errors_fp(stderr);
    return false;
  }
  const struct sockaddr_in *saddr = (const struct sockaddr_in *) addr.c_str();
  BIO_set_conn_ip(bio_, &saddr->sin_addr);
  BIO_set_conn_int_port(bio_, &portnum_);

  if (BIO_do_connect(bio_) <= 0) {
    // connection failed
    LOG(INFO) << "error connecting to " << server.str();
    ERR_print_errors_fp(stderr);
    return false;
  }
  socket_ = BIO_get_fd(bio_, NULL);
  return true;
}

bool SSLClient::SendMessage(const string& request) {
  int len = request.size();

  for (int trials = 0; trials < 10; trials++) {
    int bytes_written = BIO_write(bio_, request.c_str(), len);
    if (bytes_written > 0)
      return (bytes_written == len);
    if (!BIO_should_retry(bio_))
      return false;  // write failed
  }

  LOG(INFO) << "Too many retries have failed.";
  return false;
}


// read more data if there is data pending
NetClient::tStatus_ SSLClient::ReadMoreDataIfAvailable() {
  const int single_read_size = 65536;
  char buf[single_read_size];

  // TODO: To handle timeout, we use select() here in conjunction with
  //       BIO_read on blocking I/O.  This is not correct usage.
  //       BIO_read may still block even after select() returns, because
  //       BIO_read launches a series of multiple read/write operations.
  //
  //       According to ssl documentation, the proper way is to use
  //       non-blocking I/O.  We attempted it in revision 1639 but
  //       have not done it successfully (performance seems to be noticeably
  //       worse, especially at connection time), so we rolled back the
  //       change and still use blocking I/O for now.

  for (int trials = 0; trials < 10; trials++) {
    // wait for data with a timeout
    int max_fd = socket_ + 1;
    fd_set connections;
    FD_ZERO(&connections);
    FD_SET(socket_, &connections);
    struct timeval time_limit;
    time_limit.tv_sec = timeout_;
    time_limit.tv_usec = 0;
    select(max_fd, &connections, NULL, NULL, &time_limit);

    if (!FD_ISSET(socket_, &connections)) {
      // timeout occurs
      return D_TIMEOUT;
    }

    int bytes_read = BIO_read(bio_, buf, single_read_size);
    if (bytes_read > 0) {
      // some data has arrived

      int old_size = reply_.size();
      strutil::AppendDataToString(buf, bytes_read, max_reply_size_, &reply_);
      if (old_size < max_reply_size_ &&
          reply_.size() >= max_reply_size_)
        LOG(INFO) << "Warning: reply buffer full.  server="
               << hostname_ << ", port=" << portnum_;

      return D_OK;
    }
    else if (bytes_read == 0) {
      // server closed connection without sending any data
      return D_CLOSED;
    }
    else {
      if (!BIO_should_retry(bio_)) {
	// read failed
	return D_ERROR;
      }
      // otherwise, retry
    }
  }

  LOG(INFO) << "Too many retries have failed.";
  return D_ERROR;
}

bool SSLClient::CloseConnection() {
  clear_reply();

  if (ssl_ || bio_) {  // connection not yet closed
    VLOG(3) << "Closing connection: " << hostname_;

    ERR_remove_state(0);
    if (ssl_) {
      SSL_shutdown(ssl_);
      ssl_ = NULL;
    }
    if (bio_) {
      BIO_free_all(bio_);
      bio_ = NULL;
    }
    socket_ = -1;

    return true;
  }
  else
    return false;  // connection already closed
}

