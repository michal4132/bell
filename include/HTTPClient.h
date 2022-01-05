#ifndef BELL_HTTP_CLIENT
#define BELL_HTTP_CLIENT

#include "BellSocket.h"
#include "ByteStream.h"
#include "TCPSocket.h"
#include "platform/TLSSocket.h"
#include <map>
#include <memory>
#include <string>

#define BUF_SIZE 128

namespace bell {
class HTTPClient {
  public:
	enum HTTPMethod {
		GET,
		POST,
	};

	struct HTTPRequest {
		HTTPMethod method = HTTPMethod::GET;
		std::string url;
		const char *body = nullptr;
		const char *contentType = nullptr;
		std::map<std::string, std::string> headers;
		int maxRedirects = -1;
		std::ostream *dumpFs = nullptr;
		std::ostream *dumpRawFs = nullptr;
	};

	struct HTTPResponse : public ByteStream {
		std::shared_ptr<bell::Socket> socket;

		std::map<std::string, std::string> headers;

		uint16_t statusCode;
		size_t contentLength;
		std::string contentType;
		std::string location;
		bool isChunked = false;
		bool isGzip = false;
		bool isComplete = false;
		bool isRedirect = false;
		size_t redirectCount = 0;
		std::ostream *dumpFs = nullptr;
		std::ostream *dumpRawFs = nullptr;

		void close() override {
			socket->close();
			free(buf);
			buf = nullptr;
			bufPtr = nullptr;
		}

		void readHeaders();
		size_t read(char *dst, size_t len);
		std::string readToString();

		inline size_t skip(size_t len) override {
			return read((char *)nullptr, len);
		}
		inline size_t read(uint8_t *dst, size_t len) override {
			return read((char *)dst, len);
		}
		inline size_t size() override {
			return contentLength;
		}
		inline size_t position() override {
			return bodyRead;
		}

	  private:
		char *buf = nullptr;	// allocated buffer
		char *bufPtr = nullptr; // reading pointer within buf
		size_t bodyRead = 0;
		size_t bufRemaining = 0;
		size_t chunkRemaining = 0;
		bool isStreaming = false;
		size_t readRaw(char *dst);
		bool skipRaw(size_t len, bool dontRead = false);
	};

  private:
	static void executeImpl(const struct HTTPRequest &request, const char *url, struct HTTPResponse *&response);
	static bool readHeader(const char *&header, const char *name);

  public:
	static struct HTTPResponse *execute(const struct HTTPRequest &request);
};
} // namespace bell

#endif
