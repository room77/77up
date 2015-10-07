#include <unistd.h>
#include <thread>

#include "util/file/file.h"
#include "util/process/external.h"

FLAG_string(tmp_io_dir, "/data/tmp",
            "temporary file directory to store I/O files");

namespace External {

  // command: OS command to execute;
  //          %i will be replaced by temporary input file name
  //          %o will be replaced by temporary output file name
  bool Call(const string& command,
            const string& input, string *output, int *status) {
    output->clear();
    *status = 0;

    // construct input/output file names
    stringstream tmp_suffix;
    tmp_suffix << "tmp." << getpid() << "." << this_thread::get_id();
    string tmp_input = gFlag_tmp_io_dir + "/input." + tmp_suffix.str();
    string tmp_output = gFlag_tmp_io_dir + "/output." + tmp_suffix.str();

    // construct command
    bool has_output = false;
    string cmd = command;
    string::size_type i = cmd.find("%i");
    if (i != string::npos)
      cmd = cmd.replace(i, 2, tmp_input);
    i = cmd.find("%o");
    if (i != string::npos) {
      cmd = cmd.replace(i, 2, tmp_output);
      has_output = true;
    }

    // LOG(INFO) << "Executing: " << cmd;

    // write input to temporary file
    file::CreateDirectoryIfNecessary(tmp_input);
    fstream f(tmp_input.c_str(), ios::out);
    if (!f.good()) {
      LOG(INFO) << "Warning: Unable to create temporary file " << tmp_input;
      return false;
    }
    f << input;
    f.close();

    // execute command
    *status = system(cmd.c_str());

    bool output_read_error = false;
    if (has_output) {
      // collect output
      if (!file::ReadFileToString(tmp_output, output))
        output_read_error = true;  // cannot retrieve output
    }

    if (*status == 0 && output_read_error)
      *status = -1;

    // clean up
    remove(tmp_input.c_str());
    remove(tmp_output.c_str());

    return (*status == 0);
  }
}
