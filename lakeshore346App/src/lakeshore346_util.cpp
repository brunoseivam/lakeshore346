#include <asynPortClient.h>
#include <asynDriver.h>
#include <asynOctet.h>

std::string read_write(asynOctetClient & client, const std::string & request) {
    char response_buf[1024];
    size_t response_buf_len = sizeof(response_buf);
    size_t nread = 0;
    size_t nwritten = 0;
    int eom_reason = -1;

    asynStatus status = client.writeRead(
        request.c_str(), request.length(),
        response_buf, response_buf_len,
        &nwritten, &nread, &eom_reason
    );

    if (status != asynSuccess)
        throw std::runtime_error("Operation failed with asynStatus " + std::to_string(static_cast<int>(status)));

    if ((eom_reason & 0x02) != 0x02)
        throw std::runtime_error("writeRead did not end with EOS reason. eomReason=" + std::to_string(eom_reason));

    if (nwritten != request.length())
        throw std::runtime_error("failed to write all bytes for request: " + request);

    return response_buf;
}
