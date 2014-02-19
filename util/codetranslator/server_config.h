#ifndef _PUBLIC_UTIL_CODETRANSLATOR_SERVER_CONFIG_H_
#define _PUBLIC_UTIL_CODETRANSLATOR_SERVER_CONFIG_H_

// configuration of servers
#include <mutex>

#include "util/file/csvreader.h"

const string gGalileoServerType = "galileoserver";
const string gGeoServerType = "geoserver";
const string gMySqlServerType = "mysql";
const string gMySqlReadReplicaServerType = "mysql_read_replica";

const string gDefaultConfig_Staging = "prod_config/backend-staging.txt";
const string gDefaultConfig_Prod = "prod_config/backend.txt";
const string gDefaultConfig_Dev = "prod_config/backend-local.txt";
const string gDefaultConfig_Test = "prod_config/backend-test.txt";

extern string gFlag_server_config_file;

namespace CodeTranslator {

typedef struct {
  string server;
  int shard_id;
  string host;
  int http_port; // set to -1 if not available
  int tcp_port;  // set to -1 if not available
  CSV(server | shard_id | host | http_port | tcp_port);
} tServerConfig;

class ServerConfig {
 protected:
  ServerConfig() { Init(true); }

 public:
  ~ServerConfig() {}

  void Init(bool first_time) {
    if (gFlag_server_config_file.empty()) {
      if (SysInfo::Instance().InStaging()) {
        gFlag_server_config_file = gDefaultConfig_Staging;
        LOG(INFO) << "Using staging config file " << gFlag_server_config_file;
      }
      else if (SysInfo::Instance().InProduction()) {
        // Amazon production hostname
        gFlag_server_config_file = gDefaultConfig_Prod;
        LOG(INFO) << "Using production config file " << gFlag_server_config_file;
      }
      else if (SysInfo::Instance().InTest()) {
        gFlag_server_config_file = gDefaultConfig_Test;
        LOG(INFO) << "**************************************";
        LOG(INFO) << "***WARNING***: Using test config file " << gFlag_server_config_file;
        LOG(INFO) << "**************************************";
      }
      else {
        gFlag_server_config_file = gDefaultConfig_Dev;
        LOG(INFO) << "Using development config file " << gFlag_server_config_file;
      }
    }

    last_config_read_ = time(NULL) - 1;
    if (!first_time) {
      // during config file reload, don't crash if the file contains error
      // (just return without any change if the new file contains error)
      Collection<tServerConfig> temp;
      if (!CSVReader::ReadFromCSV(gFlag_server_config_file, ',', &temp))
        return;
    }

    {
      lock_guard<mutex> l(mutex_config_);
      storage_.Clear();
      num_shards_.clear();
      server_index_.Clear();
      server_shard_index_.Clear();
      num_galileoserver_shards_ = num_geoserver_shards_ = 0;

      ASSERT(CSVReader::ReadFromCSV(gFlag_server_config_file, ',', &storage_));
      BUILD_INDEX_ON_FIELD(storage_, server, server_index_);
      BUILD_INDEX_ON_TWO_FIELDS(storage_, server, shard_id, server_shard_index_);
      // find out how many shards each server type has
      for (Collection<tServerConfig>::const_iterator itr = storage_.begin();
           itr != storage_.end();
           ++itr) {
        unordered_map<string, int>::iterator s = num_shards_.find((*itr)->server);
        if (s == num_shards_.end())
          num_shards_[(*itr)->server] = (*itr)->shard_id + 1;
        else if (s->second <= (*itr)->shard_id) {
          // keep track of max. shard ID for each server type
          s->second = (*itr)->shard_id + 1;
        }
      }
    }
    num_galileoserver_shards_ = NumShards(gGalileoServerType);
    num_geoserver_shards_ = NumShards(gGeoServerType);
  }

  static ServerConfig& Instance() {  // singleton instance
    static ServerConfig the_one;
    return the_one;
  }

  inline int NumGalileoServerShards() const {
    return num_galileoserver_shards_;
  }
  inline int NumGeoServerShards() const {
    return num_geoserver_shards_;
  }

  inline int NumShards(const string& servertype) {
    lock_guard<mutex> l(mutex_config_);
    unordered_map<string, int>::const_iterator itr = num_shards_.find(servertype);
    int num = (itr == num_shards_.end() ? 0 : itr->second);
    return num;
  }

  inline int FindServers(const string& server, int shard_id,
                         vector<const tServerConfig *> *server_array) {
    /* TOO DANGEROUS. if a bad config files is pushed, all the live servers can die
    if (file::FileModifiedSince(gFlag_server_config_file,
                                last_config_read_))
      Init(false);  // re-read config file from disk if it has changed
    */
    lock_guard<mutex> l(mutex_config_);
    int ret = server_shard_index_.Retrieve(make_pair(server, shard_id),
                                           server_array);
    return ret;
  }

  // return a random server of this shard
  inline const tServerConfig *FindOneServer(const string& server,
                                            int shard_id) {
    vector<const tServerConfig *> array;
    FindServers(server, shard_id, &array);
    if (array.empty())
      return NULL;
    else
      return array[rand() % array.size()];
  }

 private:
  Collection<tServerConfig> storage_;
  unordered_map<string, int> num_shards_;
  Index<const char *, const tServerConfig *> server_index_;
  Index<pair<const char *, int>, const tServerConfig *> server_shard_index_;
  int num_galileoserver_shards_, num_geoserver_shards_;
  time_t last_config_read_;

  mutex mutex_config_;
};


}

#endif  // _PUBLIC_UTIL_CODETRANSLATOR_SERVER_CONFIG_H_
