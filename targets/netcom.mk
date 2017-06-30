SR_PLUGIN_LUA=1
SR_CURL_SIGNAL=0
SR_REPORTER_NUM=1024
SR_SSL_CACERT=/usr/local/ssl/certs/ca-cumulocity.pem

CC:=$(NTC_SDK_PATH)/compiler/bin/arm-ntc-linux-gnueabi-gcc
CXX:=$(NTC_SDK_PATH)/compiler/bin/arm-ntc-linux-gnueabi-g++
CXXFLAGS:=-Wall -pedantic -Wextra
CPPFLAGS:=-I$(NTC_SDK_PATH)/libstage/include -DSR_SSL_CACERT='"$(SR_SSL_CACERT)"'
LDFLAGS:=-L$(NTC_SDK_PATH)/libstage/lib
LDLIBS:=-lcurl -llua
