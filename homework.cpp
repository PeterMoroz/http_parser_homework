// homework.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <cctype>
#include <string>
#include <unordered_map>


const std::string input = "GET /wiki/http HTPP/1.1\n"
"Host: ru.wikipedia.org\n"
"User-Agent: Mozilla/5.0 (X11; U; Linux i686; ru; rv:1.9b5) Gecko/2008050509 Firefox/3.0b5\n"
"Accept: text/html\n"
"Connection: close\n";


class HttpParser
{
public:
	enum RequestType
	{
		RT_Get,
		RT_Post,
		RT_Connect,
		RT_Put,
		RT_Head,
	};

private:
	using HashType = long long;
	using HeadersContainer = std::unordered_map<HashType, std::string>;
	using RequestTypesContainer = std::unordered_map<HashType, RequestType>;

public:
	class HeadersIterator
	{
		friend class HttpParser;		

	private:
		explicit HeadersIterator(const HeadersContainer& headers)
			: _itr(headers.cbegin())
			, _end(headers.cend())
		{

		}

	public:
		const std::string& current() const { return _itr->second; }
		void next() { ++_itr; }
		bool done() const { return _itr == _end; }

	private:
		HeadersContainer::const_iterator _itr;
		HeadersContainer::const_iterator _end;
	};

public:
	HttpParser();
	~HttpParser() = default;

	void parse(const std::string& input);

	RequestType getRequestType() const { return _requestType; }
	const std::string getUrl() const { return _url; }

	const std::string& getHeaderByName(const std::string& name) const;

	HeadersIterator getHeadersIterator() const { return HeadersIterator(_headers); }

	static const char* requestTypeAsString(RequestType rt);

private:
	static HashType hash(const char* s, size_t n);

private:
	RequestType _requestType;
	std::string _url;
	HeadersContainer _headers;
	RequestTypesContainer _requestTypes;
};


HttpParser::HttpParser()
{
	HashType h = hash("GET", sizeof("GET") - 1);
	_requestTypes[h] = RT_Get;

	h = hash("POST", sizeof("POST") - 1);
	_requestTypes[h] = RT_Post;

	h = hash("CONNECT", sizeof("CONNECT") - 1);
	_requestTypes[h] = RT_Connect;

	h = hash("PUT", sizeof("PUT") - 1);
	_requestTypes[h] = RT_Put;

	h = hash("HEAD", sizeof("HEAD") - 1);
	_requestTypes[h] = RT_Head;
}

void HttpParser::parse(const std::string& input)
{
	/* There are no error checks.
	*/
	size_t p0 = 0;
	size_t p1 = input.find(' ');
	size_t n = p1 - p0;
	_requestType = _requestTypes[hash(&input[0], n)];

	p0 = p1 + 1;
	p1 = input.find(' ', p0);
	n = p1 - p0;
	_url = input.substr(p0, n);

	p0 = p1 + 1;
	p1 = input.find('\n', p0);
	while (1)
	{
		p0 = p1 + 1;
		p1 = input.find(':', p0);

		if (p1 == std::string::npos)
		{
			break;
		}

		n = p1 - p0;
		HashType key = hash(input.c_str() + p0, n);

		p0 = p1 + 2;
		p1 = input.find('\n', p0);
		n = p1 - p0;

		_headers.emplace(key, input.substr(p0, n));
	}
}


const std::string& HttpParser::getHeaderByName(const std::string& name) const
{
	HashType h = hash(name.c_str(), name.size());
	HeadersContainer::const_iterator it = _headers.find(h);
	static std::string dummy;
	return it != _headers.cend() ? it->second : dummy;
}

const char* HttpParser::requestTypeAsString(RequestType rt)
{
	switch (rt)
	{
	case RT_Get:
		return "GET";
	case RT_Post:
		return "POST";
	case RT_Connect:
		return "CONNECT";
	case RT_Put:
		return "PUT";
	case RT_Head:
		return "HEAD";
	default:
		return "<UNKNOWN>";
	}
	return "<UNKNOWN>";
}

HttpParser::HashType HttpParser::hash(const char* s, size_t n)
{
	static const int K = 31, MOD = 2023;
	HashType h = 0;
	for (size_t i = 0; i < n; i++)
	{
		char c = ::tolower(s[i]);
		int x = static_cast<int>(c - 'a' + 1);
		h = (h * K + x) % MOD;
	}
	return h;
}


int main()
{
	HttpParser httpParser;
	httpParser.parse(::input);

	std::cout << "Request type: " 
		<< HttpParser::requestTypeAsString(httpParser.getRequestType()) 
		<< "\nURL: " << httpParser.getUrl() << std::endl;

	std::cout << "Headers (values): " << std::endl;
	HttpParser::HeadersIterator headersIterator(httpParser.getHeadersIterator());

	while (!headersIterator.done())
	{
		std::cout << " - " << headersIterator.current() << std::endl;
		headersIterator.next();
	}

	std::cout << "Headers (key-value pairs): " << std::endl;
	std::cout << "Host = " << httpParser.getHeaderByName("HoSt") << std::endl;
	std::cout << "User-Agent = " << httpParser.getHeaderByName("USER-agent") << std::endl;
	std::cout << "Accept = " << httpParser.getHeaderByName("ACCEPT") << std::endl;
	std::cout << "Connection = " << httpParser.getHeaderByName("connection") << std::endl;


    return 0;
}
