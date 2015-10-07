#include "util/codetranslator/server_config.h"

#include "base/common.h"
#include "util/init/init.h"

FLAG_string(server_config_file, "", "server config info");

INIT_ADD("", []{ CodeTranslator::ServerConfig::Instance(); });
