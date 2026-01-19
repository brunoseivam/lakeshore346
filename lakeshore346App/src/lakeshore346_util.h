#pragma once

#include <asynPortClient.h>

std::string read_write(asynOctetClient & client, const std::string & request);
