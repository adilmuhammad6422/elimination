/// Json-cpp amalgated source (http://jsoncpp.sourceforge.net/).
/// It is intended to be used with #include "json/json.h"

// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////

/*
The JsonCpp library's source code, including accompanying documentation,
tests and demonstration applications, are licensed under the following
conditions...

The author (Baptiste Lepilleur) explicitly disclaims copyright in all
jurisdictions which recognize such a disclaimer. In such jurisdictions,
this software is released into the Public Domain.

In jurisdictions which do not recognize Public Domain property (e.g. Germany as of
2010), this software is Copyright (c) 2007-2010 by Baptiste Lepilleur, and is
released under the terms of the MIT License (see below).

In jurisdictions which recognize Public Domain property, the user of this
software may choose to accept it either as 1) Public Domain, 2) under the
conditions of the MIT License (see below), or 3) under the terms of dual
Public Domain/MIT License conditions described here, as they choose.

The MIT License is about as close to Public Domain as a license can get, and is
described in clear, concise terms at:

http://en.wikipedia.org/wiki/MIT_License

The full text of the MIT License follows:

========================================================================
Copyright (c) 2007-2010 Baptiste Lepilleur

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
========================================================================
(END LICENSE TEXT)

The MIT license is compatible with both the GPL and commercial
software, affording one all of the rights of Public Domain with the
minor nuisance of being required to keep the above copyright notice
and license text in the source code. Note also that by accepting the
Public Domain "license" you can re-license your copy using whatever
license you like.

*/

// //////////////////////////////////////////////////////////////////////
// End of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////






#include "JSON.h"

#ifndef JSON_IS_AMALGAMATION
#error "Compile with -I PATH_TO_JSON_DIRECTORY"
#endif


// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_tool.h
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef LIB_JSONCPP_JSON_TOOL_H_INCLUDED
#define LIB_JSONCPP_JSON_TOOL_H_INCLUDED

#ifndef NO_LOCALE_SUPPORT
#include <clocale>
#endif

/* This header provides common string manipulation support, such as UTF-8,
* portable conversion from/to string...
*
* It is an internal header that must not be exposed.
*/

namespace Json {
	static char getDecimalPoint() {
#ifdef NO_LOCALE_SUPPORT
		return '\0';
#else
		struct lconv* lc = localeconv();
		return lc ? *(lc->decimal_point) : '\0';
#endif
	}

	/// Converts a unicode code-point to UTF-8.
	static inline JSONCPP_STRING codePointToUTF8(unsigned int cp) {
		JSONCPP_STRING result;

		// based on description from http://en.wikipedia.org/wiki/UTF-8

		if (cp <= 0x7f) {
			result.resize(1);
			result[0] = static_cast<char>(cp);
		}
		else if (cp <= 0x7FF) {
			result.resize(2);
			result[1] = static_cast<char>(0x80 | (0x3f & cp));
			result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
		}
		else if (cp <= 0xFFFF) {
			result.resize(3);
			result[2] = static_cast<char>(0x80 | (0x3f & cp));
			result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
			result[0] = static_cast<char>(0xE0 | (0xf & (cp >> 12)));
		}
		else if (cp <= 0x10FFFF) {
			result.resize(4);
			result[3] = static_cast<char>(0x80 | (0x3f & cp));
			result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
			result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
			result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
		}

		return result;
	}

	/// Returns true if ch is a control character (in range [1,31]).
	static inline bool isControlCharacter(char ch) { return ch > 0 && ch <= 0x1F; }

	enum {
		/// Constant that specify the size of the buffer that must be passed to
		/// uintToString.
		uintToStringBufferSize = 3 * sizeof(LargestUInt) + 1
	};

	// Defines a char buffer for use with uintToString().
	typedef char UIntToStringBuffer[uintToStringBufferSize];

	/** Converts an unsigned integer to string.
	* @param value Unsigned interger to convert to string
	* @param current Input/Output string buffer.
	*        Must have at least uintToStringBufferSize chars free.
	*/
	static inline void uintToString(LargestUInt value, char*& current) {
		*--current = 0;
		do {
			*--current = static_cast<char>(value % 10U + static_cast<unsigned>('0'));
			value /= 10;
		} while (value != 0);
	}

	/** Change ',' to '.' everywhere in buffer.
	*
	* We had a sophisticated way, but it did not work in WinCE.
	* @see https://github.com/open-source-parsers/jsoncpp/pull/9
	*/
	static inline void fixNumericLocale(char* begin, char* end) {
		while (begin < end) {
			if (*begin == ',') {
				*begin = '.';
			}
			++begin;
		}
	}

	static inline void fixNumericLocaleInput(char* begin, char* end) {
		char decimalPoint = getDecimalPoint();
		if (decimalPoint != '\0' && decimalPoint != '.') {
			while (begin < end) {
				if (*begin == '.') {
					*begin = decimalPoint;
				}
				++begin;
			}
		}
	}

} // namespace Json {

#endif // LIB_JSONCPP_JSON_TOOL_H_INCLUDED

  // //////////////////////////////////////////////////////////////////////
  // End of content of file: src/lib_json/json_tool.h
  // //////////////////////////////////////////////////////////////////////






  // //////////////////////////////////////////////////////////////////////
  // Beginning of content of file: src/lib_json/json_reader.cpp
  // //////////////////////////////////////////////////////////////////////

  // Copyright 2007-2011 Baptiste Lepilleur
  // Distributed under MIT license, or public domain if desired and
  // recognized in your jurisdiction.
  // See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/reader.h>
#include <json/value.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <istream>
#include <sstream>
#include <memory>
#include <set>
#include <limits>

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above 
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#if !defined(__MINGW32__) && !defined(__CYGWIN__)
#define snprintf std::snprintf
#endif
#endif

#if defined(__QNXNTO__)
#define sscanf std::sscanf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
  // Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

static int const stackLimit_g = 1000;
static int       stackDepth_g = 0;  // see readValue()

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<CharReader> CharReaderPtr;
#else
	typedef std::auto_ptr<CharReader>   CharReaderPtr;
#endif

	// Implementation of class Features
	// ////////////////////////////////

	Features::Features()
		: allowComments_(true), strictRoot_(false),
		allowDroppedNullPlaceholders_(false), allowNumericKeys_(false) {}

	Features Features::all() { return Features(); }

	Features Features::strictMode() {
		Features features;
		features.allowComments_ = false;
		features.strictRoot_ = true;
		features.allowDroppedNullPlaceholders_ = false;
		features.allowNumericKeys_ = false;
		return features;
	}

	// Implementation of class Reader
	// ////////////////////////////////

	static bool containsNewLine(Reader::Location begin, Reader::Location end) {
		for (; begin < end; ++begin)
			if (*begin == '\n' || *begin == '\r')
				return true;
		return false;
	}

	// Class Reader
	// //////////////////////////////////////////////////////////////////

	Reader::Reader()
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(), features_(Features::all()),
		collectComments_() {}

	Reader::Reader(const Features& features)
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(), features_(features), collectComments_() {
	}

	bool
		Reader::parse(const std::string& document, Value& root, bool collectComments) {
		JSONCPP_STRING documentCopy(document.data(), document.data() + document.capacity());
		std::swap(documentCopy, document_);
		const char* begin = document_.c_str();
		const char* end = begin + document_.length();
		return parse(begin, end, root, collectComments);
	}

	bool Reader::parse(std::istream& sin, Value& root, bool collectComments) {
		// std::istream_iterator<char> begin(sin);
		// std::istream_iterator<char> end;
		// Those would allow streamed input from a file, if parse() were a
		// template function.

		// Since JSONCPP_STRING is reference-counted, this at least does not
		// create an extra copy.
		JSONCPP_STRING doc;
		std::getline(sin, doc, (char)EOF);
		return parse(doc.data(), doc.data() + doc.size(), root, collectComments);
	}

	bool Reader::parse(const char* beginDoc,
		const char* endDoc,
		Value& root,
		bool collectComments) {
		if (!features_.allowComments_) {
			collectComments = false;
		}

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_g = 0;  // Yes, this is bad coding, but options are limited.
		bool successful = readValue();
		Token token;
		skipCommentTokens(token);
		if (collectComments_ && !commentsBefore_.empty())
			root.setComment(commentsBefore_, commentAfter);
		if (features_.strictRoot_) {
			if (!root.isArray() && !root.isObject()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = tokenError;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				addError(
					"A valid JSON document must be either an array or an object value.",
					token);
				return false;
			}
		}
		return successful;
	}

	bool Reader::readValue() {
		// This is a non-reentrant way to support a stackLimit. Terrible!
		// But this deprecated class has a security problem: Bad input can
		// cause a seg-fault. This seems like a fair, binary-compatible way
		// to prevent the problem.
		if (stackDepth_g >= stackLimit_g) throwRuntimeError("Exceeded stackLimit in readValue().");
		++stackDepth_g;

		Token token;
		skipCommentTokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			currentValue().setComment(commentsBefore_, commentBefore);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case tokenObjectBegin:
			successful = readObject(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenArrayBegin:
			successful = readArray(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenNumber:
			successful = decodeNumber(token);
			break;
		case tokenString:
			successful = decodeString(token);
			break;
		case tokenTrue:
		{
			Value v(true);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenFalse:
		{
			Value v(false);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNull:
		{
			Value v;
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenArraySeparator:
		case tokenObjectEnd:
		case tokenArrayEnd:
			if (features_.allowDroppedNullPlaceholders_) {
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				Value v;
				currentValue().swapPayload(v);
				currentValue().setOffsetStart(current_ - begin_ - 1);
				currentValue().setOffsetLimit(current_ - begin_);
				break;
			} // Else, fall through...
		default:
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
			return addError("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &currentValue();
		}

		--stackDepth_g;
		return successful;
	}

	void Reader::skipCommentTokens(Token& token) {
		if (features_.allowComments_) {
			do {
				readToken(token);
			} while (token.type_ == tokenComment);
		}
		else {
			readToken(token);
		}
	}

	bool Reader::readToken(Token& token) {
		skipSpaces();
		token.start_ = current_;
		Char c = getNextChar();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = tokenObjectBegin;
			break;
		case '}':
			token.type_ = tokenObjectEnd;
			break;
		case '[':
			token.type_ = tokenArrayBegin;
			break;
		case ']':
			token.type_ = tokenArrayEnd;
			break;
		case '"':
			token.type_ = tokenString;
			ok = readString();
			break;
		case '/':
			token.type_ = tokenComment;
			ok = readComment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
			token.type_ = tokenNumber;
			readNumber();
			break;
		case 't':
			token.type_ = tokenTrue;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = tokenFalse;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = tokenNull;
			ok = match("ull", 3);
			break;
		case ',':
			token.type_ = tokenArraySeparator;
			break;
		case ':':
			token.type_ = tokenMemberSeparator;
			break;
		case 0:
			token.type_ = tokenEndOfStream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = tokenError;
		token.end_ = current_;
		return true;
	}

	void Reader::skipSpaces() {
		while (current_ != end_) {
			Char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool Reader::match(Location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool Reader::readComment() {
		Location commentBegin = current_ - 1;
		Char c = getNextChar();
		bool successful = false;
		if (c == '*')
			successful = readCStyleComment();
		else if (c == '/')
			successful = readCppStyleComment();
		if (!successful)
			return false;

		if (collectComments_) {
			CommentPlacement placement = commentBefore;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = commentAfterOnSameLine;
			}

			addComment(commentBegin, current_, placement);
		}
		return true;
	}

	static JSONCPP_STRING normalizeEOL(Reader::Location begin, Reader::Location end) {
		JSONCPP_STRING normalized;
		normalized.reserve(static_cast<size_t>(end - begin));
		Reader::Location current = begin;
		while (current != end) {
			char c = *current++;
			if (c == '\r') {
				if (current != end && *current == '\n')
					// convert dos EOL
					++current;
				// convert Mac EOL
				normalized += '\n';
			}
			else {
				normalized += c;
			}
		}
		return normalized;
	}

	void
		Reader::addComment(Location begin, Location end, CommentPlacement placement) {
		assert(collectComments_);
		const JSONCPP_STRING& normalized = normalizeEOL(begin, end);
		if (placement == commentAfterOnSameLine) {
			assert(lastValue_ != 0);
			lastValue_->setComment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool Reader::readCStyleComment() {
		while ((current_ + 1) < end_) {
			Char c = getNextChar();
			if (c == '*' && *current_ == '/')
				break;
		}
		return getNextChar() == '/';
	}

	bool Reader::readCppStyleComment() {
		while (current_ != end_) {
			Char c = getNextChar();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in addComment.
				if (current_ != end_ && *current_ == '\n')
					getNextChar();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	void Reader::readNumber() {
		const char *p = current_;
		char c = '0'; // stopgap for already consumed character
					  // integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : '\0';
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
	}

	bool Reader::readString() {
		Char c = '\0';
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '"')
				break;
		}
		return c == '"';
	}

	bool Reader::readObject(Token& tokenStart) {
		Token tokenName;
		JSONCPP_STRING name;
		Value init(objectValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		while (readToken(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == tokenComment && initialTokenOk)
				initialTokenOk = readToken(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == tokenString) {
				if (!decodeString(tokenName, name))
					return recoverFromError(tokenObjectEnd);
			}
			else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
				Value numberName;
				if (!decodeNumber(tokenName, numberName))
					return recoverFromError(tokenObjectEnd);
				name = JSONCPP_STRING(numberName.asCString());
			}
			else {
				break;
			}

			Token colon;
			if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
				return addErrorAndRecover(
					"Missing ':' after object member name", colon, tokenObjectEnd);
			}
			Value& value = currentValue()[name];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenObjectEnd);

			Token comma;
			if (!readToken(comma) ||
				(comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
					comma.type_ != tokenComment)) {
				return addErrorAndRecover(
					"Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == tokenComment && finalizeTokenOk)
				finalizeTokenOk = readToken(comma);
			if (comma.type_ == tokenObjectEnd)
				return true;
		}
		return addErrorAndRecover(
			"Missing '}' or object member name", tokenName, tokenObjectEnd);
	}

	bool Reader::readArray(Token& tokenStart) {
		Value init(arrayValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		skipSpaces();
		if (current_ != end_ && *current_ == ']') // empty array
		{
			Token endArray;
			readToken(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			Value& value = currentValue()[index++];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenArrayEnd);

			Token token;
			// Accept Comment after last item in the array.
			ok = readToken(token);
			while (token.type_ == tokenComment && ok) {
				ok = readToken(token);
			}
			bool badTokenType =
				(token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
			if (!ok || badTokenType) {
				return addErrorAndRecover(
					"Missing ',' or ']' in array declaration", token, tokenArrayEnd);
			}
			if (token.type_ == tokenArrayEnd)
				break;
		}
		return true;
	}

	bool Reader::decodeNumber(Token& token) {
		Value decoded;
		if (!decodeNumber(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeNumber(Token& token, Value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		Location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		Value::LargestUInt maxIntegerValue =
			isNegative ? Value::LargestUInt(Value::maxLargestInt) + 1
			: Value::maxLargestUInt;
		Value::LargestUInt threshold = maxIntegerValue / 10;
		Value::LargestUInt value = 0;
		while (current < token.end_) {
			Char c = *current++;
			if (c < '0' || c > '9')
				return decodeDouble(token, decoded);
			Value::UInt digit(static_cast<Value::UInt>(c - '0'));
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decodeDouble(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative && value == maxIntegerValue)
			decoded = Value::minLargestInt;
		else if (isNegative)
			decoded = -Value::LargestInt(value);
		else if (value <= Value::LargestUInt(Value::maxInt))
			decoded = Value::LargestInt(value);
		else
			decoded = value;
		return true;
	}

	bool Reader::decodeDouble(Token& token) {
		Value decoded;
		if (!decodeDouble(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeDouble(Token& token, Value& decoded) {
		double value = 0;
		JSONCPP_STRING buffer(token.start_, token.end_);
		JSONCPP_ISTRINGSTREAM is(buffer);
		if (!(is >> value))
			return addError("'" + JSONCPP_STRING(token.start_, token.end_) +
				"' is not a number.",
				token);
		decoded = value;
		return true;
	}

	bool Reader::decodeString(Token& token) {
		JSONCPP_STRING decoded_string;
		if (!decodeString(token, decoded_string))
			return false;
		Value decoded(decoded_string);
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool Reader::decodeString(Token& token, JSONCPP_STRING& decoded) {
		decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
		Location current = token.start_ + 1; // skip '"'
		Location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			Char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return addError("Empty escape sequence in string", token, current);
				Char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decodeUnicodeCodePoint(token, current, end, unicode))
						return false;
					decoded += codePointToUTF8(unicode);
				} break;
				default:
					return addError("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool Reader::decodeUnicodeCodePoint(Token& token,
		Location& current,
		Location end,
		unsigned int& unicode) {

		if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return addError(
					"additional six characters expected to parse unicode surrogate pair.",
					token,
					current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return addError("expecting another \\u token to begin the second half of "
					"a unicode surrogate pair",
					token,
					current);
		}
		return true;
	}

	bool Reader::decodeUnicodeEscapeSequence(Token& token,
		Location& current,
		Location end,
		unsigned int& ret_unicode) {
		if (end - current < 4)
			return addError(
				"Bad unicode escape sequence in string: four digits expected.",
				token,
				current);
		int unicode = 0;
		for (int index = 0; index < 4; ++index) {
			Char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return addError(
					"Bad unicode escape sequence in string: hexadecimal digit expected.",
					token,
					current);
		}
		ret_unicode = static_cast<unsigned int>(unicode);
		return true;
	}

	bool
		Reader::addError(const JSONCPP_STRING& message, Token& token, Location extra) {
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool Reader::recoverFromError(TokenType skipUntilToken) {
		size_t const errorCount = errors_.size();
		Token skip;
		for (;;) {
			if (!readToken(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool Reader::addErrorAndRecover(const JSONCPP_STRING& message,
		Token& token,
		TokenType skipUntilToken) {
		addError(message, token);
		return recoverFromError(skipUntilToken);
	}

	Value& Reader::currentValue() { return *(nodes_.top()); }

	Reader::Char Reader::getNextChar() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void Reader::getLocationLineAndColumn(Location location,
		int& line,
		int& column) const {
		Location current = begin_;
		Location lastLineStart = current;
		line = 0;
		while (current < location && current != end_) {
			Char c = *current++;
			if (c == '\r') {
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n') {
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	JSONCPP_STRING Reader::getLocationLineAndColumn(Location location) const {
		int line, column;
		getLocationLineAndColumn(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	// Deprecated. Preserved for backward compatibility
	JSONCPP_STRING Reader::getFormatedErrorMessages() const {
		return getFormattedErrorMessages();
	}

	JSONCPP_STRING Reader::getFormattedErrorMessages() const {
		JSONCPP_STRING formattedMessage;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			formattedMessage +=
				"* " + getLocationLineAndColumn(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<Reader::StructuredError> Reader::getStructuredErrors() const {
		std::vector<Reader::StructuredError> allErrors;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			Reader::StructuredError structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}
		return allErrors;
	}

	bool Reader::pushError(const Value& value, const JSONCPP_STRING& message) {
		ptrdiff_t const length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = end_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool Reader::pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra) {
		ptrdiff_t const length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length
			|| extra.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = begin_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.getOffsetStart();
		errors_.push_back(info);
		return true;
	}

	bool Reader::good() const {
		return !errors_.size();
	}

	// exact copy of Features
	class OurFeatures {
	public:
		static OurFeatures all();
		bool allowComments_;
		bool strictRoot_;
		bool allowDroppedNullPlaceholders_;
		bool allowNumericKeys_;
		bool allowSingleQuotes_;
		bool failIfExtra_;
		bool rejectDupKeys_;
		bool allowSpecialFloats_;
		int stackLimit_;
	};  // OurFeatures

		// exact copy of Implementation of class Features
		// ////////////////////////////////

	OurFeatures OurFeatures::all() { return OurFeatures(); }

	// Implementation of class Reader
	// ////////////////////////////////

	// exact copy of Reader, renamed to OurReader
	class OurReader {
	public:
		typedef char Char;
		typedef const Char* Location;
		struct StructuredError {
			ptrdiff_t offset_start;
			ptrdiff_t offset_limit;
			JSONCPP_STRING message;
		};

		OurReader(OurFeatures const& features);
		bool parse(const char* beginDoc,
			const char* endDoc,
			Value& root,
			bool collectComments = true);
		JSONCPP_STRING getFormattedErrorMessages() const;
		std::vector<StructuredError> getStructuredErrors() const;
		bool pushError(const Value& value, const JSONCPP_STRING& message);
		bool pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra);
		bool good() const;

	private:
		OurReader(OurReader const&);  // no impl
		void operator=(OurReader const&);  // no impl

		enum TokenType {
			tokenEndOfStream = 0,
			tokenObjectBegin,
			tokenObjectEnd,
			tokenArrayBegin,
			tokenArrayEnd,
			tokenString,
			tokenNumber,
			tokenTrue,
			tokenFalse,
			tokenNull,
			tokenNaN,
			tokenPosInf,
			tokenNegInf,
			tokenArraySeparator,
			tokenMemberSeparator,
			tokenComment,
			tokenError
		};

		class Token {
		public:
			TokenType type_;
			Location start_;
			Location end_;
		};

		class ErrorInfo {
		public:
			Token token_;
			JSONCPP_STRING message_;
			Location extra_;
		};

		typedef std::deque<ErrorInfo> Errors;

		bool readToken(Token& token);
		void skipSpaces();
		bool match(Location pattern, int patternLength);
		bool readComment();
		bool readCStyleComment();
		bool readCppStyleComment();
		bool readString();
		bool readStringSingleQuote();
		bool readNumber(bool checkInf);
		bool readValue();
		bool readObject(Token& token);
		bool readArray(Token& token);
		bool decodeNumber(Token& token);
		bool decodeNumber(Token& token, Value& decoded);
		bool decodeString(Token& token);
		bool decodeString(Token& token, JSONCPP_STRING& decoded);
		bool decodeDouble(Token& token);
		bool decodeDouble(Token& token, Value& decoded);
		bool decodeUnicodeCodePoint(Token& token,
			Location& current,
			Location end,
			unsigned int& unicode);
		bool decodeUnicodeEscapeSequence(Token& token,
			Location& current,
			Location end,
			unsigned int& unicode);
		bool addError(const JSONCPP_STRING& message, Token& token, Location extra = 0);
		bool recoverFromError(TokenType skipUntilToken);
		bool addErrorAndRecover(const JSONCPP_STRING& message,
			Token& token,
			TokenType skipUntilToken);
		void skipUntilSpace();
		Value& currentValue();
		Char getNextChar();
		void
			getLocationLineAndColumn(Location location, int& line, int& column) const;
		JSONCPP_STRING getLocationLineAndColumn(Location location) const;
		void addComment(Location begin, Location end, CommentPlacement placement);
		void skipCommentTokens(Token& token);

		typedef std::stack<Value*> Nodes;
		Nodes nodes_;
		Errors errors_;
		JSONCPP_STRING document_;
		Location begin_;
		Location end_;
		Location current_;
		Location lastValueEnd_;
		Value* lastValue_;
		JSONCPP_STRING commentsBefore_;
		int stackDepth_;

		OurFeatures const features_;
		bool collectComments_;
	};  // OurReader

		// complete copy of Read impl, for OurReader

	OurReader::OurReader(OurFeatures const& features)
		: errors_(), document_(), begin_(), end_(), current_(), lastValueEnd_(),
		lastValue_(), commentsBefore_(),
		stackDepth_(0),
		features_(features), collectComments_() {
	}

	bool OurReader::parse(const char* beginDoc,
		const char* endDoc,
		Value& root,
		bool collectComments) {
		if (!features_.allowComments_) {
			collectComments = false;
		}

		begin_ = beginDoc;
		end_ = endDoc;
		collectComments_ = collectComments;
		current_ = begin_;
		lastValueEnd_ = 0;
		lastValue_ = 0;
		commentsBefore_ = "";
		errors_.clear();
		while (!nodes_.empty())
			nodes_.pop();
		nodes_.push(&root);

		stackDepth_ = 0;
		bool successful = readValue();
		Token token;
		skipCommentTokens(token);
		if (features_.failIfExtra_) {
			if ((features_.strictRoot_ || token.type_ != tokenError) && token.type_ != tokenEndOfStream) {
				addError("Extra non-whitespace after JSON value.", token);
				return false;
			}
		}
		if (collectComments_ && !commentsBefore_.empty())
			root.setComment(commentsBefore_, commentAfter);
		if (features_.strictRoot_) {
			if (!root.isArray() && !root.isObject()) {
				// Set error location to start of doc, ideally should be first token found
				// in doc
				token.type_ = tokenError;
				token.start_ = beginDoc;
				token.end_ = endDoc;
				addError(
					"A valid JSON document must be either an array or an object value.",
					token);
				return false;
			}
		}
		return successful;
	}

	bool OurReader::readValue() {
		if (stackDepth_ >= features_.stackLimit_) throwRuntimeError("Exceeded stackLimit in readValue().");
		++stackDepth_;
		Token token;
		skipCommentTokens(token);
		bool successful = true;

		if (collectComments_ && !commentsBefore_.empty()) {
			currentValue().setComment(commentsBefore_, commentBefore);
			commentsBefore_ = "";
		}

		switch (token.type_) {
		case tokenObjectBegin:
			successful = readObject(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenArrayBegin:
			successful = readArray(token);
			currentValue().setOffsetLimit(current_ - begin_);
			break;
		case tokenNumber:
			successful = decodeNumber(token);
			break;
		case tokenString:
			successful = decodeString(token);
			break;
		case tokenTrue:
		{
			Value v(true);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenFalse:
		{
			Value v(false);
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNull:
		{
			Value v;
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNaN:
		{
			Value v(std::numeric_limits<double>::quiet_NaN());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenPosInf:
		{
			Value v(std::numeric_limits<double>::infinity());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenNegInf:
		{
			Value v(-std::numeric_limits<double>::infinity());
			currentValue().swapPayload(v);
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
		}
		break;
		case tokenArraySeparator:
		case tokenObjectEnd:
		case tokenArrayEnd:
			if (features_.allowDroppedNullPlaceholders_) {
				// "Un-read" the current token and mark the current value as a null
				// token.
				current_--;
				Value v;
				currentValue().swapPayload(v);
				currentValue().setOffsetStart(current_ - begin_ - 1);
				currentValue().setOffsetLimit(current_ - begin_);
				break;
			} // else, fall through ...
		default:
			currentValue().setOffsetStart(token.start_ - begin_);
			currentValue().setOffsetLimit(token.end_ - begin_);
			return addError("Syntax error: value, object or array expected.", token);
		}

		if (collectComments_) {
			lastValueEnd_ = current_;
			lastValue_ = &currentValue();
		}

		--stackDepth_;
		return successful;
	}

	void OurReader::skipCommentTokens(Token& token) {
		if (features_.allowComments_) {
			do {
				readToken(token);
			} while (token.type_ == tokenComment);
		}
		else {
			readToken(token);
		}
	}

	bool OurReader::readToken(Token& token) {
		skipSpaces();
		token.start_ = current_;
		Char c = getNextChar();
		bool ok = true;
		switch (c) {
		case '{':
			token.type_ = tokenObjectBegin;
			break;
		case '}':
			token.type_ = tokenObjectEnd;
			break;
		case '[':
			token.type_ = tokenArrayBegin;
			break;
		case ']':
			token.type_ = tokenArrayEnd;
			break;
		case '"':
			token.type_ = tokenString;
			ok = readString();
			break;
		case '\'':
			if (features_.allowSingleQuotes_) {
				token.type_ = tokenString;
				ok = readStringSingleQuote();
				break;
			} // else continue
		case '/':
			token.type_ = tokenComment;
			ok = readComment();
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token.type_ = tokenNumber;
			readNumber(false);
			break;
		case '-':
			if (readNumber(true)) {
				token.type_ = tokenNumber;
			}
			else {
				token.type_ = tokenNegInf;
				ok = features_.allowSpecialFloats_ && match("nfinity", 7);
			}
			break;
		case 't':
			token.type_ = tokenTrue;
			ok = match("rue", 3);
			break;
		case 'f':
			token.type_ = tokenFalse;
			ok = match("alse", 4);
			break;
		case 'n':
			token.type_ = tokenNull;
			ok = match("ull", 3);
			break;
		case 'N':
			if (features_.allowSpecialFloats_) {
				token.type_ = tokenNaN;
				ok = match("aN", 2);
			}
			else {
				ok = false;
			}
			break;
		case 'I':
			if (features_.allowSpecialFloats_) {
				token.type_ = tokenPosInf;
				ok = match("nfinity", 7);
			}
			else {
				ok = false;
			}
			break;
		case ',':
			token.type_ = tokenArraySeparator;
			break;
		case ':':
			token.type_ = tokenMemberSeparator;
			break;
		case 0:
			token.type_ = tokenEndOfStream;
			break;
		default:
			ok = false;
			break;
		}
		if (!ok)
			token.type_ = tokenError;
		token.end_ = current_;
		return true;
	}

	void OurReader::skipSpaces() {
		while (current_ != end_) {
			Char c = *current_;
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				++current_;
			else
				break;
		}
	}

	bool OurReader::match(Location pattern, int patternLength) {
		if (end_ - current_ < patternLength)
			return false;
		int index = patternLength;
		while (index--)
			if (current_[index] != pattern[index])
				return false;
		current_ += patternLength;
		return true;
	}

	bool OurReader::readComment() {
		Location commentBegin = current_ - 1;
		Char c = getNextChar();
		bool successful = false;
		if (c == '*')
			successful = readCStyleComment();
		else if (c == '/')
			successful = readCppStyleComment();
		if (!successful)
			return false;

		if (collectComments_) {
			CommentPlacement placement = commentBefore;
			if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
				if (c != '*' || !containsNewLine(commentBegin, current_))
					placement = commentAfterOnSameLine;
			}

			addComment(commentBegin, current_, placement);
		}
		return true;
	}

	void
		OurReader::addComment(Location begin, Location end, CommentPlacement placement) {
		assert(collectComments_);
		const JSONCPP_STRING& normalized = normalizeEOL(begin, end);
		if (placement == commentAfterOnSameLine) {
			assert(lastValue_ != 0);
			lastValue_->setComment(normalized, placement);
		}
		else {
			commentsBefore_ += normalized;
		}
	}

	bool OurReader::readCStyleComment() {
		while ((current_ + 1) < end_) {
			Char c = getNextChar();
			if (c == '*' && *current_ == '/')
				break;
		}
		return getNextChar() == '/';
	}

	bool OurReader::readCppStyleComment() {
		while (current_ != end_) {
			Char c = getNextChar();
			if (c == '\n')
				break;
			if (c == '\r') {
				// Consume DOS EOL. It will be normalized in addComment.
				if (current_ != end_ && *current_ == '\n')
					getNextChar();
				// Break on Moc OS 9 EOL.
				break;
			}
		}
		return true;
	}

	bool OurReader::readNumber(bool checkInf) {
		const char *p = current_;
		if (checkInf && p != end_ && *p == 'I') {
			current_ = ++p;
			return false;
		}
		char c = '0'; // stopgap for already consumed character
					  // integral part
		while (c >= '0' && c <= '9')
			c = (current_ = p) < end_ ? *p++ : '\0';
		// fractional part
		if (c == '.') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		// exponential part
		if (c == 'e' || c == 'E') {
			c = (current_ = p) < end_ ? *p++ : '\0';
			if (c == '+' || c == '-')
				c = (current_ = p) < end_ ? *p++ : '\0';
			while (c >= '0' && c <= '9')
				c = (current_ = p) < end_ ? *p++ : '\0';
		}
		return true;
	}
	bool OurReader::readString() {
		Char c = 0;
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '"')
				break;
		}
		return c == '"';
	}


	bool OurReader::readStringSingleQuote() {
		Char c = 0;
		while (current_ != end_) {
			c = getNextChar();
			if (c == '\\')
				getNextChar();
			else if (c == '\'')
				break;
		}
		return c == '\'';
	}

	bool OurReader::readObject(Token& tokenStart) {
		Token tokenName;
		JSONCPP_STRING name;
		Value init(objectValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		while (readToken(tokenName)) {
			bool initialTokenOk = true;
			while (tokenName.type_ == tokenComment && initialTokenOk)
				initialTokenOk = readToken(tokenName);
			if (!initialTokenOk)
				break;
			if (tokenName.type_ == tokenObjectEnd && name.empty()) // empty object
				return true;
			name = "";
			if (tokenName.type_ == tokenString) {
				if (!decodeString(tokenName, name))
					return recoverFromError(tokenObjectEnd);
			}
			else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
				Value numberName;
				if (!decodeNumber(tokenName, numberName))
					return recoverFromError(tokenObjectEnd);
				name = numberName.asString();
			}
			else {
				break;
			}

			Token colon;
			if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
				return addErrorAndRecover(
					"Missing ':' after object member name", colon, tokenObjectEnd);
			}
			if (name.length() >= (1U << 30)) throwRuntimeError("keylength >= 2^30");
			if (features_.rejectDupKeys_ && currentValue().isMember(name)) {
				JSONCPP_STRING msg = "Duplicate key: '" + name + "'";
				return addErrorAndRecover(
					msg, tokenName, tokenObjectEnd);
			}
			Value& value = currentValue()[name];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenObjectEnd);

			Token comma;
			if (!readToken(comma) ||
				(comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
					comma.type_ != tokenComment)) {
				return addErrorAndRecover(
					"Missing ',' or '}' in object declaration", comma, tokenObjectEnd);
			}
			bool finalizeTokenOk = true;
			while (comma.type_ == tokenComment && finalizeTokenOk)
				finalizeTokenOk = readToken(comma);
			if (comma.type_ == tokenObjectEnd)
				return true;
		}
		return addErrorAndRecover(
			"Missing '}' or object member name", tokenName, tokenObjectEnd);
	}

	bool OurReader::readArray(Token& tokenStart) {
		Value init(arrayValue);
		currentValue().swapPayload(init);
		currentValue().setOffsetStart(tokenStart.start_ - begin_);
		skipSpaces();
		if (current_ != end_ && *current_ == ']') // empty array
		{
			Token endArray;
			readToken(endArray);
			return true;
		}
		int index = 0;
		for (;;) {
			Value& value = currentValue()[index++];
			nodes_.push(&value);
			bool ok = readValue();
			nodes_.pop();
			if (!ok) // error already set
				return recoverFromError(tokenArrayEnd);

			Token token;
			// Accept Comment after last item in the array.
			ok = readToken(token);
			while (token.type_ == tokenComment && ok) {
				ok = readToken(token);
			}
			bool badTokenType =
				(token.type_ != tokenArraySeparator && token.type_ != tokenArrayEnd);
			if (!ok || badTokenType) {
				return addErrorAndRecover(
					"Missing ',' or ']' in array declaration", token, tokenArrayEnd);
			}
			if (token.type_ == tokenArrayEnd)
				break;
		}
		return true;
	}

	bool OurReader::decodeNumber(Token& token) {
		Value decoded;
		if (!decodeNumber(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeNumber(Token& token, Value& decoded) {
		// Attempts to parse the number as an integer. If the number is
		// larger than the maximum supported value of an integer then
		// we decode the number as a double.
		Location current = token.start_;
		bool isNegative = *current == '-';
		if (isNegative)
			++current;
		// TODO: Help the compiler do the div and mod at compile time or get rid of them.
		Value::LargestUInt maxIntegerValue =
			isNegative ? Value::LargestUInt(-Value::minLargestInt)
			: Value::maxLargestUInt;
		Value::LargestUInt threshold = maxIntegerValue / 10;
		Value::LargestUInt value = 0;
		while (current < token.end_) {
			Char c = *current++;
			if (c < '0' || c > '9')
				return decodeDouble(token, decoded);
			Value::UInt digit(static_cast<Value::UInt>(c - '0'));
			if (value >= threshold) {
				// We've hit or exceeded the max value divided by 10 (rounded down). If
				// a) we've only just touched the limit, b) this is the last digit, and
				// c) it's small enough to fit in that rounding delta, we're okay.
				// Otherwise treat this number as a double to avoid overflow.
				if (value > threshold || current != token.end_ ||
					digit > maxIntegerValue % 10) {
					return decodeDouble(token, decoded);
				}
			}
			value = value * 10 + digit;
		}
		if (isNegative)
			decoded = -Value::LargestInt(value);
		else if (value <= Value::LargestUInt(Value::maxInt))
			decoded = Value::LargestInt(value);
		else
			decoded = value;
		return true;
	}

	bool OurReader::decodeDouble(Token& token) {
		Value decoded;
		if (!decodeDouble(token, decoded))
			return false;
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeDouble(Token& token, Value& decoded) {
		double value = 0;
		const int bufferSize = 32;
		int count;
		ptrdiff_t const length = token.end_ - token.start_;

		// Sanity check to avoid buffer overflow exploits.
		if (length < 0) {
			return addError("Unable to parse token length", token);
		}
		size_t const ulength = static_cast<size_t>(length);

		// Avoid using a string constant for the format control string given to
		// sscanf, as this can cause hard to debug crashes on OS X. See here for more
		// info:
		//
		//     http://developer.apple.com/library/mac/#DOCUMENTATION/DeveloperTools/gcc-4.0.1/gcc/Incompatibilities.html
		char format[] = "%lf";

		if (length <= bufferSize) {
			Char buffer[bufferSize + 1];
			memcpy(buffer, token.start_, ulength);
			buffer[length] = 0;
			fixNumericLocaleInput(buffer, buffer + length);
			count = sscanf(buffer, format, &value);
		}
		else {
			JSONCPP_STRING buffer(token.start_, token.end_);
			count = sscanf(buffer.c_str(), format, &value);
		}

		if (count != 1)
			return addError("'" + JSONCPP_STRING(token.start_, token.end_) +
				"' is not a number.",
				token);
		decoded = value;
		return true;
	}

	bool OurReader::decodeString(Token& token) {
		JSONCPP_STRING decoded_string;
		if (!decodeString(token, decoded_string))
			return false;
		Value decoded(decoded_string);
		currentValue().swapPayload(decoded);
		currentValue().setOffsetStart(token.start_ - begin_);
		currentValue().setOffsetLimit(token.end_ - begin_);
		return true;
	}

	bool OurReader::decodeString(Token& token, JSONCPP_STRING& decoded) {
		decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
		Location current = token.start_ + 1; // skip '"'
		Location end = token.end_ - 1;       // do not include '"'
		while (current != end) {
			Char c = *current++;
			if (c == '"')
				break;
			else if (c == '\\') {
				if (current == end)
					return addError("Empty escape sequence in string", token, current);
				Char escape = *current++;
				switch (escape) {
				case '"':
					decoded += '"';
					break;
				case '/':
					decoded += '/';
					break;
				case '\\':
					decoded += '\\';
					break;
				case 'b':
					decoded += '\b';
					break;
				case 'f':
					decoded += '\f';
					break;
				case 'n':
					decoded += '\n';
					break;
				case 'r':
					decoded += '\r';
					break;
				case 't':
					decoded += '\t';
					break;
				case 'u': {
					unsigned int unicode;
					if (!decodeUnicodeCodePoint(token, current, end, unicode))
						return false;
					decoded += codePointToUTF8(unicode);
				} break;
				default:
					return addError("Bad escape sequence in string", token, current);
				}
			}
			else {
				decoded += c;
			}
		}
		return true;
	}

	bool OurReader::decodeUnicodeCodePoint(Token& token,
		Location& current,
		Location end,
		unsigned int& unicode) {

		if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
			return false;
		if (unicode >= 0xD800 && unicode <= 0xDBFF) {
			// surrogate pairs
			if (end - current < 6)
				return addError(
					"additional six characters expected to parse unicode surrogate pair.",
					token,
					current);
			unsigned int surrogatePair;
			if (*(current++) == '\\' && *(current++) == 'u') {
				if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
					unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
				}
				else
					return false;
			}
			else
				return addError("expecting another \\u token to begin the second half of "
					"a unicode surrogate pair",
					token,
					current);
		}
		return true;
	}

	bool OurReader::decodeUnicodeEscapeSequence(Token& token,
		Location& current,
		Location end,
		unsigned int& ret_unicode) {
		if (end - current < 4)
			return addError(
				"Bad unicode escape sequence in string: four digits expected.",
				token,
				current);
		int unicode = 0;
		for (int index = 0; index < 4; ++index) {
			Char c = *current++;
			unicode *= 16;
			if (c >= '0' && c <= '9')
				unicode += c - '0';
			else if (c >= 'a' && c <= 'f')
				unicode += c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				unicode += c - 'A' + 10;
			else
				return addError(
					"Bad unicode escape sequence in string: hexadecimal digit expected.",
					token,
					current);
		}
		ret_unicode = static_cast<unsigned int>(unicode);
		return true;
	}

	bool
		OurReader::addError(const JSONCPP_STRING& message, Token& token, Location extra) {
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = extra;
		errors_.push_back(info);
		return false;
	}

	bool OurReader::recoverFromError(TokenType skipUntilToken) {
		size_t errorCount = errors_.size();
		Token skip;
		for (;;) {
			if (!readToken(skip))
				errors_.resize(errorCount); // discard errors caused by recovery
			if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
				break;
		}
		errors_.resize(errorCount);
		return false;
	}

	bool OurReader::addErrorAndRecover(const JSONCPP_STRING& message,
		Token& token,
		TokenType skipUntilToken) {
		addError(message, token);
		return recoverFromError(skipUntilToken);
	}

	Value& OurReader::currentValue() { return *(nodes_.top()); }

	OurReader::Char OurReader::getNextChar() {
		if (current_ == end_)
			return 0;
		return *current_++;
	}

	void OurReader::getLocationLineAndColumn(Location location,
		int& line,
		int& column) const {
		Location current = begin_;
		Location lastLineStart = current;
		line = 0;
		while (current < location && current != end_) {
			Char c = *current++;
			if (c == '\r') {
				if (*current == '\n')
					++current;
				lastLineStart = current;
				++line;
			}
			else if (c == '\n') {
				lastLineStart = current;
				++line;
			}
		}
		// column & line start at 1
		column = int(location - lastLineStart) + 1;
		++line;
	}

	JSONCPP_STRING OurReader::getLocationLineAndColumn(Location location) const {
		int line, column;
		getLocationLineAndColumn(location, line, column);
		char buffer[18 + 16 + 16 + 1];
		snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
		return buffer;
	}

	JSONCPP_STRING OurReader::getFormattedErrorMessages() const {
		JSONCPP_STRING formattedMessage;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			formattedMessage +=
				"* " + getLocationLineAndColumn(error.token_.start_) + "\n";
			formattedMessage += "  " + error.message_ + "\n";
			if (error.extra_)
				formattedMessage +=
				"See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
		}
		return formattedMessage;
	}

	std::vector<OurReader::StructuredError> OurReader::getStructuredErrors() const {
		std::vector<OurReader::StructuredError> allErrors;
		for (Errors::const_iterator itError = errors_.begin();
			itError != errors_.end();
			++itError) {
			const ErrorInfo& error = *itError;
			OurReader::StructuredError structured;
			structured.offset_start = error.token_.start_ - begin_;
			structured.offset_limit = error.token_.end_ - begin_;
			structured.message = error.message_;
			allErrors.push_back(structured);
		}
		return allErrors;
	}

	bool OurReader::pushError(const Value& value, const JSONCPP_STRING& message) {
		ptrdiff_t length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = end_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = 0;
		errors_.push_back(info);
		return true;
	}

	bool OurReader::pushError(const Value& value, const JSONCPP_STRING& message, const Value& extra) {
		ptrdiff_t length = end_ - begin_;
		if (value.getOffsetStart() > length
			|| value.getOffsetLimit() > length
			|| extra.getOffsetLimit() > length)
			return false;
		Token token;
		token.type_ = tokenError;
		token.start_ = begin_ + value.getOffsetStart();
		token.end_ = begin_ + value.getOffsetLimit();
		ErrorInfo info;
		info.token_ = token;
		info.message_ = message;
		info.extra_ = begin_ + extra.getOffsetStart();
		errors_.push_back(info);
		return true;
	}

	bool OurReader::good() const {
		return !errors_.size();
	}


	class OurCharReader : public CharReader {
		bool const collectComments_;
		OurReader reader_;
	public:
		OurCharReader(
			bool collectComments,
			OurFeatures const& features)
			: collectComments_(collectComments)
			, reader_(features)
		{}
		bool parse(
			char const* beginDoc, char const* endDoc,
			Value* root, JSONCPP_STRING* errs) JSONCPP_OVERRIDE {
			bool ok = reader_.parse(beginDoc, endDoc, *root, collectComments_);
			if (errs) {
				*errs = reader_.getFormattedErrorMessages();
			}
			return ok;
		}
	};

	CharReaderBuilder::CharReaderBuilder()
	{
		setDefaults(&settings_);
	}
	CharReaderBuilder::~CharReaderBuilder()
	{}
	CharReader* CharReaderBuilder::newCharReader() const
	{
		bool collectComments = settings_["collectComments"].asBool();
		OurFeatures features = OurFeatures::all();
		features.allowComments_ = settings_["allowComments"].asBool();
		features.strictRoot_ = settings_["strictRoot"].asBool();
		features.allowDroppedNullPlaceholders_ = settings_["allowDroppedNullPlaceholders"].asBool();
		features.allowNumericKeys_ = settings_["allowNumericKeys"].asBool();
		features.allowSingleQuotes_ = settings_["allowSingleQuotes"].asBool();
		features.stackLimit_ = settings_["stackLimit"].asInt();
		features.failIfExtra_ = settings_["failIfExtra"].asBool();
		features.rejectDupKeys_ = settings_["rejectDupKeys"].asBool();
		features.allowSpecialFloats_ = settings_["allowSpecialFloats"].asBool();
		return new OurCharReader(collectComments, features);
	}
	static void getValidReaderKeys(std::set<JSONCPP_STRING>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("collectComments");
		valid_keys->insert("allowComments");
		valid_keys->insert("strictRoot");
		valid_keys->insert("allowDroppedNullPlaceholders");
		valid_keys->insert("allowNumericKeys");
		valid_keys->insert("allowSingleQuotes");
		valid_keys->insert("stackLimit");
		valid_keys->insert("failIfExtra");
		valid_keys->insert("rejectDupKeys");
		valid_keys->insert("allowSpecialFloats");
	}
	bool CharReaderBuilder::validate(Json::Value* invalid) const
	{
		Json::Value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		Json::Value& inv = *invalid;
		std::set<JSONCPP_STRING> valid_keys;
		getValidReaderKeys(&valid_keys);
		Value::Members keys = settings_.getMemberNames();
		size_t n = keys.size();
		for (size_t i = 0; i < n; ++i) {
			JSONCPP_STRING const& key = keys[i];
			if (valid_keys.find(key) == valid_keys.end()) {
				inv[key] = settings_[key];
			}
		}
		return 0u == inv.size();
	}
	Value& CharReaderBuilder::operator[](JSONCPP_STRING key)
	{
		return settings_[key];
	}
	// static
	void CharReaderBuilder::strictMode(Json::Value* settings)
	{
		//! [CharReaderBuilderStrictMode]
		(*settings)["allowComments"] = false;
		(*settings)["strictRoot"] = true;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = true;
		(*settings)["rejectDupKeys"] = true;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderStrictMode]
	}
	// static
	void CharReaderBuilder::setDefaults(Json::Value* settings)
	{
		//! [CharReaderBuilderDefaults]
		(*settings)["collectComments"] = true;
		(*settings)["allowComments"] = true;
		(*settings)["strictRoot"] = false;
		(*settings)["allowDroppedNullPlaceholders"] = false;
		(*settings)["allowNumericKeys"] = false;
		(*settings)["allowSingleQuotes"] = false;
		(*settings)["stackLimit"] = 1000;
		(*settings)["failIfExtra"] = false;
		(*settings)["rejectDupKeys"] = false;
		(*settings)["allowSpecialFloats"] = false;
		//! [CharReaderBuilderDefaults]
	}

	//////////////////////////////////
	// global functions

	bool parseFromStream(
		CharReader::Factory const& fact, JSONCPP_ISTREAM& sin,
		Value* root, JSONCPP_STRING* errs)
	{
		JSONCPP_OSTRINGSTREAM ssin;
		ssin << sin.rdbuf();
		JSONCPP_STRING doc = ssin.str();
		char const* begin = doc.data();
		char const* end = begin + doc.size();
		// Note that we do not actually need a null-terminator.
		CharReaderPtr const reader(fact.newCharReader());
		return reader->parse(begin, end, root, errs);
	}

	JSONCPP_ISTREAM& operator>>(JSONCPP_ISTREAM& sin, Value& root) {
		CharReaderBuilder b;
		JSONCPP_STRING errs;
		bool ok = parseFromStream(b, sin, &root, &errs);
		if (!ok) {
			fprintf(stderr,
				"Error from reader: %s",
				errs.c_str());

			throwRuntimeError(errs);
		}
		return sin;
	}

} // namespace Json

  // //////////////////////////////////////////////////////////////////////
  // End of content of file: src/lib_json/json_reader.cpp
  // //////////////////////////////////////////////////////////////////////






  // //////////////////////////////////////////////////////////////////////
  // Beginning of content of file: src/lib_json/json_valueiterator.inl
  // //////////////////////////////////////////////////////////////////////

  // Copyright 2007-2010 Baptiste Lepilleur
  // Distributed under MIT license, or public domain if desired and
  // recognized in your jurisdiction.
  // See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

  // included by json_value.cpp

namespace Json {

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueIteratorBase
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueIteratorBase::ValueIteratorBase()
		: current_(), isNull_(true) {
	}

	ValueIteratorBase::ValueIteratorBase(
		const Value::ObjectValues::iterator& current)
		: current_(current), isNull_(false) {}

	Value& ValueIteratorBase::deref() const {
		return current_->second;
	}

	void ValueIteratorBase::increment() {
		++current_;
	}

	void ValueIteratorBase::decrement() {
		--current_;
	}

	ValueIteratorBase::difference_type
		ValueIteratorBase::computeDistance(const SelfType& other) const {
#ifdef JSON_USE_CPPTL_SMALLMAP
		return other.current_ - current_;
#else
		// Iterator for null value are initialized using the default
		// constructor, which initialize current_ to the default
		// std::map::iterator. As begin() and end() are two instance
		// of the default std::map::iterator, they can not be compared.
		// To allow this, we handle this comparison specifically.
		if (isNull_ && other.isNull_) {
			return 0;
		}

		// Usage of std::distance is not portable (does not compile with Sun Studio 12
		// RogueWave STL,
		// which is the one used by default).
		// Using a portable hand-made version for non random iterator instead:
		//   return difference_type( std::distance( current_, other.current_ ) );
		difference_type myDistance = 0;
		for (Value::ObjectValues::iterator it = current_; it != other.current_;
			++it) {
			++myDistance;
		}
		return myDistance;
#endif
	}

	bool ValueIteratorBase::isEqual(const SelfType& other) const {
		if (isNull_) {
			return other.isNull_;
		}
		return current_ == other.current_;
	}

	void ValueIteratorBase::copy(const SelfType& other) {
		current_ = other.current_;
		isNull_ = other.isNull_;
	}

	Value ValueIteratorBase::key() const {
		const Value::CZString czstring = (*current_).first;
		if (czstring.data()) {
			if (czstring.isStaticString())
				return Value(StaticString(czstring.data()));
			return Value(czstring.data(), czstring.data() + czstring.length());
		}
		return Value(czstring.index());
	}

	UInt ValueIteratorBase::index() const {
		const Value::CZString czstring = (*current_).first;
		if (!czstring.data())
			return czstring.index();
		return Value::UInt(-1);
	}

	JSONCPP_STRING ValueIteratorBase::name() const {
		char const* keey;
		char const* end;
		keey = memberName(&end);
		if (!keey) return JSONCPP_STRING();
		return JSONCPP_STRING(keey, end);
	}

	char const* ValueIteratorBase::memberName() const {
		const char* cname = (*current_).first.data();
		return cname ? cname : "";
	}

	char const* ValueIteratorBase::memberName(char const** end) const {
		const char* cname = (*current_).first.data();
		if (!cname) {
			*end = NULL;
			return NULL;
		}
		*end = cname + (*current_).first.length();
		return cname;
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueConstIterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueConstIterator::ValueConstIterator() {}

	ValueConstIterator::ValueConstIterator(
		const Value::ObjectValues::iterator& current)
		: ValueIteratorBase(current) {}

	ValueConstIterator::ValueConstIterator(ValueIterator const& other)
		: ValueIteratorBase(other) {}

	ValueConstIterator& ValueConstIterator::
		operator=(const ValueIteratorBase& other) {
		copy(other);
		return *this;
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class ValueIterator
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	ValueIterator::ValueIterator() {}

	ValueIterator::ValueIterator(const Value::ObjectValues::iterator& current)
		: ValueIteratorBase(current) {}

	ValueIterator::ValueIterator(const ValueConstIterator& other)
		: ValueIteratorBase(other) {
		throwRuntimeError("ConstIterator to Iterator should never be allowed.");
	}

	ValueIterator::ValueIterator(const ValueIterator& other)
		: ValueIteratorBase(other) {}

	ValueIterator& ValueIterator::operator=(const SelfType& other) {
		copy(other);
		return *this;
	}

} // namespace Json

  // //////////////////////////////////////////////////////////////////////
  // End of content of file: src/lib_json/json_valueiterator.inl
  // //////////////////////////////////////////////////////////////////////






  // //////////////////////////////////////////////////////////////////////
  // Beginning of content of file: src/lib_json/json_value.cpp
  // //////////////////////////////////////////////////////////////////////

  // Copyright 2011 Baptiste Lepilleur
  // Distributed under MIT license, or public domain if desired and
  // recognized in your jurisdiction.
  // See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/value.h>
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <math.h>
#include <sstream>
#include <utility>
#include <cstring>
#include <cassert>
#ifdef JSON_USE_CPPTL
#include <cpptl/conststring.h>
#endif
#include <cstddef> // size_t
#include <algorithm> // min()

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace Json {

	// This is a walkaround to avoid the static initialization of Value::null.
	// kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
	// 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif
	//static const unsigned char ALIGNAS(8) kNull[sizeof(Value)] = { 0 };
	//const unsigned char& kNullRef = kNull[0];
	//const Value& Value::null = reinterpret_cast<const Value&>(kNullRef);
	//const Value& Value::nullRef = null;

	// static
	Value const& Value::nullSingleton()
	{
		static Value const nullStatic;
		return nullStatic;
	}

	// for backwards compatibility, we'll leave these global references around, but DO NOT
	// use them in JSONCPP library code any more!
	Value const& Value::null = Value::nullSingleton();
	Value const& Value::nullRef = Value::nullSingleton();

	const Int Value::minInt = Int(~(UInt(-1) / 2));
	const Int Value::maxInt = Int(UInt(-1) / 2);
	const UInt Value::maxUInt = UInt(-1);
#if defined(JSON_HAS_INT64)
	const Int64 Value::minInt64 = Int64(~(UInt64(-1) / 2));
	const Int64 Value::maxInt64 = Int64(UInt64(-1) / 2);
	const UInt64 Value::maxUInt64 = UInt64(-1);
	// The constant is hard-coded because some compiler have trouble
	// converting Value::maxUInt64 to a double correctly (AIX/xlC).
	// Assumes that UInt64 is a 64 bits integer.
	static const double maxUInt64AsDouble = 18446744073709551615.0;
#endif // defined(JSON_HAS_INT64)
	const LargestInt Value::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
	const LargestInt Value::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
	const LargestUInt Value::maxLargestUInt = LargestUInt(-1);

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	template <typename T, typename U>
	static inline bool InRange(double d, T min, U max) {
		// The casts can lose precision, but we are looking only for
		// an approximate range. Might fail on edge cases though. ~cdunn
		//return d >= static_cast<double>(min) && d <= static_cast<double>(max);
		return d >= min && d <= max;
	}
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
	static inline double integerToDouble(Json::UInt64 value) {
		return static_cast<double>(Int64(value / 2)) * 2.0 + static_cast<double>(Int64(value & 1));
	}

	template <typename T> static inline double integerToDouble(T value) {
		return static_cast<double>(value);
	}

	template <typename T, typename U>
	static inline bool InRange(double d, T min, U max) {
		return d >= integerToDouble(min) && d <= integerToDouble(max);
	}
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

	/** Duplicates the specified string value.
	* @param value Pointer to the string to duplicate. Must be zero-terminated if
	*              length is "unknown".
	* @param length Length of the value. if equals to unknown, then it will be
	*               computed using strlen(value).
	* @return Pointer on the duplicate instance of string.
	*/
	static inline char* duplicateStringValue(const char* value,
		size_t length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		if (length >= static_cast<size_t>(Value::maxInt))
			length = Value::maxInt - 1;

		char* newString = static_cast<char*>(malloc(length + 1));
		if (newString == NULL) {
			throwRuntimeError(
				"in Json::Value::duplicateStringValue(): "
				"Failed to allocate string value buffer");
		}
		memcpy(newString, value, length);
		newString[length] = 0;
		return newString;
	}

	/* Record the length as a prefix.
	*/
	static inline char* duplicateAndPrefixStringValue(
		const char* value,
		unsigned int length)
	{
		// Avoid an integer overflow in the call to malloc below by limiting length
		// to a sane value.
		JSON_ASSERT_MESSAGE(length <= static_cast<unsigned>(Value::maxInt) - sizeof(unsigned) - 1U,
			"in Json::Value::duplicateAndPrefixStringValue(): "
			"length too big for prefixing");
		unsigned actualLength = length + static_cast<unsigned>(sizeof(unsigned)) + 1U;
		char* newString = static_cast<char*>(malloc(actualLength));
		if (newString == 0) {
			throwRuntimeError(
				"in Json::Value::duplicateAndPrefixStringValue(): "
				"Failed to allocate string value buffer");
		}
		*reinterpret_cast<unsigned*>(newString) = length;
		memcpy(newString + sizeof(unsigned), value, length);
		newString[actualLength - 1U] = 0; // to avoid buffer over-run accidents by users later
		return newString;
	}
	inline static void decodePrefixedString(
		bool isPrefixed, char const* prefixed,
		unsigned* length, char const** value)
	{
		if (!isPrefixed) {
			*length = static_cast<unsigned>(strlen(prefixed));
			*value = prefixed;
		}
		else {
			*length = *reinterpret_cast<unsigned const*>(prefixed);
			*value = prefixed + sizeof(unsigned);
		}
	}
	/** Free the string duplicated by duplicateStringValue()/duplicateAndPrefixStringValue().
	*/
#if JSONCPP_USING_SECURE_MEMORY
	static inline void releasePrefixedStringValue(char* value) {
		unsigned length = 0;
		char const* valueDecoded;
		decodePrefixedString(true, value, &length, &valueDecoded);
		size_t const size = sizeof(unsigned) + length + 1U;
		memset(value, 0, size);
		free(value);
	}
	static inline void releaseStringValue(char* value, unsigned length) {
		// length==0 => we allocated the strings memory
		size_t size = (length == 0) ? strlen(value) : length;
		memset(value, 0, size);
		free(value);
	}
#else // !JSONCPP_USING_SECURE_MEMORY
	static inline void releasePrefixedStringValue(char* value) {
		free(value);
	}
	static inline void releaseStringValue(char* value, unsigned) {
		free(value);
	}
#endif // JSONCPP_USING_SECURE_MEMORY

} // namespace Json

  // //////////////////////////////////////////////////////////////////
  // //////////////////////////////////////////////////////////////////
  // //////////////////////////////////////////////////////////////////
  // ValueInternals...
  // //////////////////////////////////////////////////////////////////
  // //////////////////////////////////////////////////////////////////
  // //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

	Exception::Exception(JSONCPP_STRING const& msg)
		: msg_(msg)
	{}
	Exception::~Exception() throw()
	{}
	char const* Exception::what() const throw()
	{
		return msg_.c_str();
	}
	RuntimeError::RuntimeError(JSONCPP_STRING const& msg)
		: Exception(msg)
	{}
	LogicError::LogicError(JSONCPP_STRING const& msg)
		: Exception(msg)
	{}
	JSONCPP_NORETURN void throwRuntimeError(JSONCPP_STRING const& msg)
	{
		throw RuntimeError(msg);
	}
	JSONCPP_NORETURN void throwLogicError(JSONCPP_STRING const& msg)
	{
		throw LogicError(msg);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::CommentInfo
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	Value::CommentInfo::CommentInfo() : comment_(0)
	{}

	Value::CommentInfo::~CommentInfo() {
		if (comment_)
			releaseStringValue(comment_, 0u);
	}

	void Value::CommentInfo::setComment(const char* text, size_t len) {
		if (comment_) {
			releaseStringValue(comment_, 0u);
			comment_ = 0;
		}
		JSON_ASSERT(text != 0);
		JSON_ASSERT_MESSAGE(
			text[0] == '\0' || text[0] == '/',
			"in Json::Value::setComment(): Comments must start with /");
		// It seems that /**/ style comments are acceptable as well.
		comment_ = duplicateStringValue(text, len);
	}

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::CZString
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	// Notes: policy_ indicates if the string was allocated when
	// a string is stored.

	Value::CZString::CZString(ArrayIndex aindex) : cstr_(0), index_(aindex) {}

	Value::CZString::CZString(char const* str, unsigned ulength, DuplicationPolicy allocate)
		: cstr_(str) {
		// allocate != duplicate
		storage_.policy_ = allocate & 0x3;
		storage_.length_ = ulength & 0x3FFFFFFF;
	}

	Value::CZString::CZString(const CZString& other) {
		cstr_ = (other.storage_.policy_ != noDuplication && other.cstr_ != 0
			? duplicateStringValue(other.cstr_, other.storage_.length_)
			: other.cstr_);
		storage_.policy_ = static_cast<unsigned>(other.cstr_
			? (static_cast<DuplicationPolicy>(other.storage_.policy_) == noDuplication
				? noDuplication : duplicate)
			: static_cast<DuplicationPolicy>(other.storage_.policy_)) & 3U;
		storage_.length_ = other.storage_.length_;
	}

#if JSON_HAS_RVALUE_REFERENCES
	Value::CZString::CZString(CZString&& other)
		: cstr_(other.cstr_), index_(other.index_) {
		other.cstr_ = nullptr;
	}
#endif

	Value::CZString::~CZString() {
		if (cstr_ && storage_.policy_ == duplicate) {
			releaseStringValue(const_cast<char*>(cstr_), storage_.length_ + 1u); //+1 for null terminating character for sake of completeness but not actually necessary
		}
	}

	void Value::CZString::swap(CZString& other) {
		std::swap(cstr_, other.cstr_);
		std::swap(index_, other.index_);
	}

	Value::CZString& Value::CZString::operator=(CZString other) {
		swap(other);
		return *this;
	}

	bool Value::CZString::operator<(const CZString& other) const {
		if (!cstr_) return index_ < other.index_;
		//return strcmp(cstr_, other.cstr_) < 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		unsigned min_len = std::min(this_len, other_len);
		JSON_ASSERT(this->cstr_ && other.cstr_);
		int comp = memcmp(this->cstr_, other.cstr_, min_len);
		if (comp < 0) return true;
		if (comp > 0) return false;
		return (this_len < other_len);
	}

	bool Value::CZString::operator==(const CZString& other) const {
		if (!cstr_) return index_ == other.index_;
		//return strcmp(cstr_, other.cstr_) == 0;
		// Assume both are strings.
		unsigned this_len = this->storage_.length_;
		unsigned other_len = other.storage_.length_;
		if (this_len != other_len) return false;
		JSON_ASSERT(this->cstr_ && other.cstr_);
		int comp = memcmp(this->cstr_, other.cstr_, this_len);
		return comp == 0;
	}

	ArrayIndex Value::CZString::index() const { return index_; }

	//const char* Value::CZString::c_str() const { return cstr_; }
	const char* Value::CZString::data() const { return cstr_; }
	unsigned Value::CZString::length() const { return storage_.length_; }
	bool Value::CZString::isStaticString() const { return storage_.policy_ == noDuplication; }

	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// class Value::Value
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////
	// //////////////////////////////////////////////////////////////////

	/*! \internal Default constructor initialization must be equivalent to:
	* memset( this, 0, sizeof(Value) )
	* This optimization is used in ValueInternalMap fast allocator.
	*/
	Value::Value(ValueType vtype) {
		static char const empty[] = "";
		initBasic(vtype);
		switch (vtype) {
		case nullValue:
			break;
		case intValue:
		case uintValue:
			value_.int_ = 0;
			break;
		case realValue:
			value_.real_ = 0.0;
			break;
		case stringValue:
			// allocated_ == false, so this is safe.
			value_.string_ = const_cast<char*>(static_cast<char const*>(empty));
			break;
		case arrayValue:
		case objectValue:
			value_.map_ = new ObjectValues();
			break;
		case booleanValue:
			value_.bool_ = false;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}
	}

	Value::Value(Int value) {
		initBasic(intValue);
		value_.int_ = value;
	}

	Value::Value(UInt value) {
		initBasic(uintValue);
		value_.uint_ = value;
	}
#if defined(JSON_HAS_INT64)
	Value::Value(Int64 value) {
		initBasic(intValue);
		value_.int_ = value;
	}
	Value::Value(UInt64 value) {
		initBasic(uintValue);
		value_.uint_ = value;
	}
#endif // defined(JSON_HAS_INT64)

	Value::Value(double value) {
		initBasic(realValue);
		value_.real_ = value;
	}

	Value::Value(const char* value) {
		initBasic(stringValue, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(strlen(value)));
	}

	Value::Value(const char* beginValue, const char* endValue) {
		initBasic(stringValue, true);
		value_.string_ =
			duplicateAndPrefixStringValue(beginValue, static_cast<unsigned>(endValue - beginValue));
	}

	Value::Value(const JSONCPP_STRING& value) {
		initBasic(stringValue, true);
		value_.string_ =
			duplicateAndPrefixStringValue(value.data(), static_cast<unsigned>(value.length()));
	}

	Value::Value(const StaticString& value) {
		initBasic(stringValue);
		value_.string_ = const_cast<char*>(value.c_str());
	}

#ifdef JSON_USE_CPPTL
	Value::Value(const CppTL::ConstString& value) {
		initBasic(stringValue, true);
		value_.string_ = duplicateAndPrefixStringValue(value, static_cast<unsigned>(value.length()));
	}
#endif

	Value::Value(bool value) {
		initBasic(booleanValue);
		value_.bool_ = value;
	}

	Value::Value(Value const& other)
		: type_(other.type_), allocated_(false)
		,
		comments_(0), start_(other.start_), limit_(other.limit_)
	{
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
			value_ = other.value_;
			break;
		case stringValue:
			if (other.value_.string_ && other.allocated_) {
				unsigned len;
				char const* str;
				decodePrefixedString(other.allocated_, other.value_.string_,
					&len, &str);
				value_.string_ = duplicateAndPrefixStringValue(str, len);
				allocated_ = true;
			}
			else {
				value_.string_ = other.value_.string_;
				allocated_ = false;
			}
			break;
		case arrayValue:
		case objectValue:
			value_.map_ = new ObjectValues(*other.value_.map_);
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		if (other.comments_) {
			comments_ = new CommentInfo[numberOfCommentPlacement];
			for (int comment = 0; comment < numberOfCommentPlacement; ++comment) {
				const CommentInfo& otherComment = other.comments_[comment];
				if (otherComment.comment_)
					comments_[comment].setComment(
						otherComment.comment_, strlen(otherComment.comment_));
			}
		}
	}

#if JSON_HAS_RVALUE_REFERENCES
	// Move constructor
	Value::Value(Value&& other) {
		initBasic(nullValue);
		swap(other);
	}
#endif

	Value::~Value() {
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
			break;
		case stringValue:
			if (allocated_)
				releasePrefixedStringValue(value_.string_);
			break;
		case arrayValue:
		case objectValue:
			delete value_.map_;
			break;
		default:
			JSON_ASSERT_UNREACHABLE;
		}

		delete[] comments_;

		value_.uint_ = 0;
	}

	Value& Value::operator=(Value other) {
		swap(other);
		return *this;
	}

	void Value::swapPayload(Value& other) {
		ValueType temp = type_;
		type_ = other.type_;
		other.type_ = temp;
		std::swap(value_, other.value_);
		int temp2 = allocated_;
		allocated_ = other.allocated_;
		other.allocated_ = temp2 & 0x1;
	}

	void Value::swap(Value& other) {
		swapPayload(other);
		std::swap(comments_, other.comments_);
		std::swap(start_, other.start_);
		std::swap(limit_, other.limit_);
	}

	ValueType Value::type() const { return type_; }

	int Value::compare(const Value& other) const {
		if (*this < other)
			return -1;
		if (*this > other)
			return 1;
		return 0;
	}

	bool Value::operator<(const Value& other) const {
		int typeDelta = type_ - other.type_;
		if (typeDelta)
			return typeDelta < 0 ? true : false;
		switch (type_) {
		case nullValue:
			return false;
		case intValue:
			return value_.int_ < other.value_.int_;
		case uintValue:
			return value_.uint_ < other.value_.uint_;
		case realValue:
			return value_.real_ < other.value_.real_;
		case booleanValue:
			return value_.bool_ < other.value_.bool_;
		case stringValue:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
				if (other.value_.string_) return true;
				else return false;
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			unsigned min_len = std::min(this_len, other_len);
			JSON_ASSERT(this_str && other_str);
			int comp = memcmp(this_str, other_str, min_len);
			if (comp < 0) return true;
			if (comp > 0) return false;
			return (this_len < other_len);
		}
		case arrayValue:
		case objectValue: {
			int delta = int(value_.map_->size() - other.value_.map_->size());
			if (delta)
				return delta < 0;
			return (*value_.map_) < (*other.value_.map_);
		}
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool Value::operator<=(const Value& other) const { return !(other < *this); }

	bool Value::operator>=(const Value& other) const { return !(*this < other); }

	bool Value::operator>(const Value& other) const { return other < *this; }

	bool Value::operator==(const Value& other) const {
		// if ( type_ != other.type_ )
		// GCC 2.95.3 says:
		// attempt to take address of bit-field structure member `Json::Value::type_'
		// Beats me, but a temp solves the problem.
		int temp = other.type_;
		if (type_ != temp)
			return false;
		switch (type_) {
		case nullValue:
			return true;
		case intValue:
			return value_.int_ == other.value_.int_;
		case uintValue:
			return value_.uint_ == other.value_.uint_;
		case realValue:
			return value_.real_ == other.value_.real_;
		case booleanValue:
			return value_.bool_ == other.value_.bool_;
		case stringValue:
		{
			if ((value_.string_ == 0) || (other.value_.string_ == 0)) {
				return (value_.string_ == other.value_.string_);
			}
			unsigned this_len;
			unsigned other_len;
			char const* this_str;
			char const* other_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			decodePrefixedString(other.allocated_, other.value_.string_, &other_len, &other_str);
			if (this_len != other_len) return false;
			JSON_ASSERT(this_str && other_str);
			int comp = memcmp(this_str, other_str, this_len);
			return comp == 0;
		}
		case arrayValue:
		case objectValue:
			return value_.map_->size() == other.value_.map_->size() &&
				(*value_.map_) == (*other.value_.map_);
		default:
			JSON_ASSERT_UNREACHABLE;
		}
		return false; // unreachable
	}

	bool Value::operator!=(const Value& other) const { return !(*this == other); }

	const char* Value::asCString() const {
		JSON_ASSERT_MESSAGE(type_ == stringValue,
			"in Json::Value::asCString(): requires stringValue");
		if (value_.string_ == 0) return 0;
		unsigned this_len;
		char const* this_str;
		decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
		return this_str;
	}

#if JSONCPP_USING_SECURE_MEMORY
	unsigned Value::getCStringLength() const {
		JSON_ASSERT_MESSAGE(type_ == stringValue,
			"in Json::Value::asCString(): requires stringValue");
		if (value_.string_ == 0) return 0;
		unsigned this_len;
		char const* this_str;
		decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
		return this_len;
	}
#endif

	bool Value::getString(char const** str, char const** cend) const {
		if (type_ != stringValue) return false;
		if (value_.string_ == 0) return false;
		unsigned length;
		decodePrefixedString(this->allocated_, this->value_.string_, &length, str);
		*cend = *str + length;
		return true;
	}

	JSONCPP_STRING Value::asString() const {
		switch (type_) {
		case nullValue:
			return "";
		case stringValue:
		{
			if (value_.string_ == 0) return "";
			unsigned this_len;
			char const* this_str;
			decodePrefixedString(this->allocated_, this->value_.string_, &this_len, &this_str);
			return JSONCPP_STRING(this_str, this_len);
		}
		case booleanValue:
			return value_.bool_ ? "true" : "false";
		case intValue:
			return valueToString(value_.int_);
		case uintValue:
			return valueToString(value_.uint_);
		case realValue:
			return valueToString(value_.real_);
		default:
			JSON_FAIL_MESSAGE("Type is not convertible to string");
		}
	}

#ifdef JSON_USE_CPPTL
	CppTL::ConstString Value::asConstString() const {
		unsigned len;
		char const* str;
		decodePrefixedString(allocated_, value_.string_,
			&len, &str);
		return CppTL::ConstString(str, len);
	}
#endif

	Value::Int Value::asInt() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of Int range");
			return Int(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of Int range");
			return Int(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt),
				"double out of Int range");
			return Int(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to Int.");
	}

	Value::UInt Value::asUInt() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
			return UInt(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
			return UInt(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt),
				"double out of UInt range");
			return UInt(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
	}

#if defined(JSON_HAS_INT64)

	Value::Int64 Value::asInt64() const {
		switch (type_) {
		case intValue:
			return Int64(value_.int_);
		case uintValue:
			JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
			return Int64(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64),
				"double out of Int64 range");
			return Int64(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
	}

	Value::UInt64 Value::asUInt64() const {
		switch (type_) {
		case intValue:
			JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
			return UInt64(value_.int_);
		case uintValue:
			return UInt64(value_.uint_);
		case realValue:
			JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64),
				"double out of UInt64 range");
			return UInt64(value_.real_);
		case nullValue:
			return 0;
		case booleanValue:
			return value_.bool_ ? 1 : 0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
	}
#endif // if defined(JSON_HAS_INT64)

	LargestInt Value::asLargestInt() const {
#if defined(JSON_NO_INT64)
		return asInt();
#else
		return asInt64();
#endif
	}

	LargestUInt Value::asLargestUInt() const {
#if defined(JSON_NO_INT64)
		return asUInt();
#else
		return asUInt64();
#endif
	}

	double Value::asDouble() const {
		switch (type_) {
		case intValue:
			return static_cast<double>(value_.int_);
		case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<double>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return integerToDouble(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case realValue:
			return value_.real_;
		case nullValue:
			return 0.0;
		case booleanValue:
			return value_.bool_ ? 1.0 : 0.0;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to double.");
	}

	float Value::asFloat() const {
		switch (type_) {
		case intValue:
			return static_cast<float>(value_.int_);
		case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
			// This can fail (silently?) if the value is bigger than MAX_FLOAT.
			return static_cast<float>(integerToDouble(value_.uint_));
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
		case realValue:
			return static_cast<float>(value_.real_);
		case nullValue:
			return 0.0;
		case booleanValue:
			return value_.bool_ ? 1.0f : 0.0f;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to float.");
	}

	bool Value::asBool() const {
		switch (type_) {
		case booleanValue:
			return value_.bool_;
		case nullValue:
			return false;
		case intValue:
			return value_.int_ ? true : false;
		case uintValue:
			return value_.uint_ ? true : false;
		case realValue:
			// This is kind of strange. Not recommended.
			return (value_.real_ != 0.0) ? true : false;
		default:
			break;
		}
		JSON_FAIL_MESSAGE("Value is not convertible to bool.");
	}

	bool Value::isConvertibleTo(ValueType other) const {
		switch (other) {
		case nullValue:
			return (isNumeric() && asDouble() == 0.0) ||
				(type_ == booleanValue && value_.bool_ == false) ||
				(type_ == stringValue && asString() == "") ||
				(type_ == arrayValue && value_.map_->size() == 0) ||
				(type_ == objectValue && value_.map_->size() == 0) ||
				type_ == nullValue;
		case intValue:
			return isInt() ||
				(type_ == realValue && InRange(value_.real_, minInt, maxInt)) ||
				type_ == booleanValue || type_ == nullValue;
		case uintValue:
			return isUInt() ||
				(type_ == realValue && InRange(value_.real_, 0, maxUInt)) ||
				type_ == booleanValue || type_ == nullValue;
		case realValue:
			return isNumeric() || type_ == booleanValue || type_ == nullValue;
		case booleanValue:
			return isNumeric() || type_ == booleanValue || type_ == nullValue;
		case stringValue:
			return isNumeric() || type_ == booleanValue || type_ == stringValue ||
				type_ == nullValue;
		case arrayValue:
			return type_ == arrayValue || type_ == nullValue;
		case objectValue:
			return type_ == objectValue || type_ == nullValue;
		}
		JSON_ASSERT_UNREACHABLE;
		return false;
	}

	/// Number of values in array or object
	ArrayIndex Value::size() const {
		switch (type_) {
		case nullValue:
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
		case stringValue:
			return 0;
		case arrayValue: // size of the array is highest index + 1
			if (!value_.map_->empty()) {
				ObjectValues::const_iterator itLast = value_.map_->end();
				--itLast;
				return (*itLast).first.index() + 1;
			}
			return 0;
		case objectValue:
			return ArrayIndex(value_.map_->size());
		}
		JSON_ASSERT_UNREACHABLE;
		return 0; // unreachable;
	}

	bool Value::empty() const {
		if (isNull() || isArray() || isObject())
			return size() == 0u;
		else
			return false;
	}

	bool Value::operator!() const { return isNull(); }

	void Value::clear() {
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue ||
			type_ == objectValue,
			"in Json::Value::clear(): requires complex value");
		start_ = 0;
		limit_ = 0;
		switch (type_) {
		case arrayValue:
		case objectValue:
			value_.map_->clear();
			break;
		default:
			break;
		}
	}

	void Value::resize(ArrayIndex newSize) {
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == arrayValue,
			"in Json::Value::resize(): requires arrayValue");
		if (type_ == nullValue)
			*this = Value(arrayValue);
		ArrayIndex oldSize = size();
		if (newSize == 0)
			clear();
		else if (newSize > oldSize)
			(*this)[newSize - 1];
		else {
			for (ArrayIndex index = newSize; index < oldSize; ++index) {
				value_.map_->erase(index);
			}
			JSON_ASSERT(size() == newSize);
		}
	}

	Value& Value::operator[](ArrayIndex index) {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == arrayValue,
			"in Json::Value::operator[](ArrayIndex): requires arrayValue");
		if (type_ == nullValue)
			*this = Value(arrayValue);
		CZString key(index);
		ObjectValues::iterator it = value_.map_->lower_bound(key);
		if (it != value_.map_->end() && (*it).first == key)
			return (*it).second;

		ObjectValues::value_type defaultValue(key, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		return (*it).second;
	}

	Value& Value::operator[](int index) {
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in Json::Value::operator[](int index): index cannot be negative");
		return (*this)[ArrayIndex(index)];
	}

	const Value& Value::operator[](ArrayIndex index) const {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == arrayValue,
			"in Json::Value::operator[](ArrayIndex)const: requires arrayValue");
		if (type_ == nullValue)
			return nullSingleton();
		CZString key(index);
		ObjectValues::const_iterator it = value_.map_->find(key);
		if (it == value_.map_->end())
			return nullSingleton();
		return (*it).second;
	}

	const Value& Value::operator[](int index) const {
		JSON_ASSERT_MESSAGE(
			index >= 0,
			"in Json::Value::operator[](int index) const: index cannot be negative");
		return (*this)[ArrayIndex(index)];
	}

	void Value::initBasic(ValueType vtype, bool allocated) {
		type_ = vtype;
		allocated_ = allocated;
		comments_ = 0;
		start_ = 0;
		limit_ = 0;
	}

	// Access an object value by name, create a null member if it does not exist.
	// @pre Type of '*this' is object or null.
	// @param key is null-terminated.
	Value& Value::resolveReference(const char* key) {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::resolveReference(): requires objectValue");
		if (type_ == nullValue)
			*this = Value(objectValue);
		CZString actualKey(
			key, static_cast<unsigned>(strlen(key)), CZString::noDuplication); // NOTE!
		ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		ObjectValues::value_type defaultValue(actualKey, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		Value& value = (*it).second;
		return value;
	}

	// @param key is not null-terminated.
	Value& Value::resolveReference(char const* key, char const* cend)
	{
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::resolveReference(key, end): requires objectValue");
		if (type_ == nullValue)
			*this = Value(objectValue);
		CZString actualKey(
			key, static_cast<unsigned>(cend - key), CZString::duplicateOnCopy);
		ObjectValues::iterator it = value_.map_->lower_bound(actualKey);
		if (it != value_.map_->end() && (*it).first == actualKey)
			return (*it).second;

		ObjectValues::value_type defaultValue(actualKey, nullSingleton());
		it = value_.map_->insert(it, defaultValue);
		Value& value = (*it).second;
		return value;
	}

	Value Value::get(ArrayIndex index, const Value& defaultValue) const {
		const Value* value = &((*this)[index]);
		return value == &nullSingleton() ? defaultValue : *value;
	}

	bool Value::isValidIndex(ArrayIndex index) const { return index < size(); }

	Value const* Value::find(char const* key, char const* cend) const
	{
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::find(key, end, found): requires objectValue or nullValue");
		if (type_ == nullValue) return NULL;
		CZString actualKey(key, static_cast<unsigned>(cend - key), CZString::noDuplication);
		ObjectValues::const_iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end()) return NULL;
		return &(*it).second;
	}
	const Value& Value::operator[](const char* key) const
	{
		Value const* found = find(key, key + strlen(key));
		if (!found) return nullSingleton();
		return *found;
	}
	Value const& Value::operator[](JSONCPP_STRING const& key) const
	{
		Value const* found = find(key.data(), key.data() + key.length());
		if (!found) return nullSingleton();
		return *found;
	}

	Value& Value::operator[](const char* key) {
		return resolveReference(key, key + strlen(key));
	}

	Value& Value::operator[](const JSONCPP_STRING& key) {
		return resolveReference(key.data(), key.data() + key.length());
	}

	Value& Value::operator[](const StaticString& key) {
		return resolveReference(key.c_str());
	}

#ifdef JSON_USE_CPPTL
	Value& Value::operator[](const CppTL::ConstString& key) {
		return resolveReference(key.c_str(), key.end_c_str());
	}
	Value const& Value::operator[](CppTL::ConstString const& key) const
	{
		Value const* found = find(key.c_str(), key.end_c_str());
		if (!found) return nullSingleton();
		return *found;
	}
#endif

	Value& Value::append(const Value& value) { return (*this)[size()] = value; }

	Value Value::get(char const* key, char const* cend, Value const& defaultValue) const
	{
		Value const* found = find(key, cend);
		return !found ? defaultValue : *found;
	}
	Value Value::get(char const* key, Value const& defaultValue) const
	{
		return get(key, key + strlen(key), defaultValue);
	}
	Value Value::get(JSONCPP_STRING const& key, Value const& defaultValue) const
	{
		return get(key.data(), key.data() + key.length(), defaultValue);
	}


	bool Value::removeMember(const char* key, const char* cend, Value* removed)
	{
		if (type_ != objectValue) {
			return false;
		}
		CZString actualKey(key, static_cast<unsigned>(cend - key), CZString::noDuplication);
		ObjectValues::iterator it = value_.map_->find(actualKey);
		if (it == value_.map_->end())
			return false;
		*removed = it->second;
		value_.map_->erase(it);
		return true;
	}
	bool Value::removeMember(const char* key, Value* removed)
	{
		return removeMember(key, key + strlen(key), removed);
	}
	bool Value::removeMember(JSONCPP_STRING const& key, Value* removed)
	{
		return removeMember(key.data(), key.data() + key.length(), removed);
	}
	Value Value::removeMember(const char* key)
	{
		JSON_ASSERT_MESSAGE(type_ == nullValue || type_ == objectValue,
			"in Json::Value::removeMember(): requires objectValue");
		if (type_ == nullValue)
			return nullSingleton();

		Value removed;  // null
		removeMember(key, key + strlen(key), &removed);
		return removed; // still null if removeMember() did nothing
	}
	Value Value::removeMember(const JSONCPP_STRING& key)
	{
		return removeMember(key.c_str());
	}

	bool Value::removeIndex(ArrayIndex index, Value* removed) {
		if (type_ != arrayValue) {
			return false;
		}
		CZString key(index);
		ObjectValues::iterator it = value_.map_->find(key);
		if (it == value_.map_->end()) {
			return false;
		}
		*removed = it->second;
		ArrayIndex oldSize = size();
		// shift left all items left, into the place of the "removed"
		for (ArrayIndex i = index; i < (oldSize - 1); ++i) {
			CZString keey(i);
			(*value_.map_)[keey] = (*this)[i + 1];
		}
		// erase the last one ("leftover")
		CZString keyLast(oldSize - 1);
		ObjectValues::iterator itLast = value_.map_->find(keyLast);
		value_.map_->erase(itLast);
		return true;
	}

#ifdef JSON_USE_CPPTL
	Value Value::get(const CppTL::ConstString& key,
		const Value& defaultValue) const {
		return get(key.c_str(), key.end_c_str(), defaultValue);
	}
#endif

	bool Value::isMember(char const* key, char const* cend) const
	{
		Value const* value = find(key, cend);
		return NULL != value;
	}
	bool Value::isMember(char const* key) const
	{
		return isMember(key, key + strlen(key));
	}
	bool Value::isMember(JSONCPP_STRING const& key) const
	{
		return isMember(key.data(), key.data() + key.length());
	}

#ifdef JSON_USE_CPPTL
	bool Value::isMember(const CppTL::ConstString& key) const {
		return isMember(key.c_str(), key.end_c_str());
	}
#endif

	Value::Members Value::getMemberNames() const {
		JSON_ASSERT_MESSAGE(
			type_ == nullValue || type_ == objectValue,
			"in Json::Value::getMemberNames(), value must be objectValue");
		if (type_ == nullValue)
			return Value::Members();
		Members members;
		members.reserve(value_.map_->size());
		ObjectValues::const_iterator it = value_.map_->begin();
		ObjectValues::const_iterator itEnd = value_.map_->end();
		for (; it != itEnd; ++it) {
			members.push_back(JSONCPP_STRING((*it).first.data(),
				(*it).first.length()));
		}
		return members;
	}
	//
	//# ifdef JSON_USE_CPPTL
	// EnumMemberNames
	// Value::enumMemberNames() const
	//{
	//   if ( type_ == objectValue )
	//   {
	//      return CppTL::Enum::any(  CppTL::Enum::transform(
	//         CppTL::Enum::keys( *(value_.map_), CppTL::Type<const CZString &>() ),
	//         MemberNamesTransform() ) );
	//   }
	//   return EnumMemberNames();
	//}
	//
	//
	// EnumValues
	// Value::enumValues() const
	//{
	//   if ( type_ == objectValue  ||  type_ == arrayValue )
	//      return CppTL::Enum::anyValues( *(value_.map_),
	//                                     CppTL::Type<const Value &>() );
	//   return EnumValues();
	//}
	//
	//# endif

	static bool IsIntegral(double d) {
		double integral_part;
		return modf(d, &integral_part) == 0.0;
	}

	bool Value::isNull() const { return type_ == nullValue; }

	bool Value::isBool() const { return type_ == booleanValue; }

	bool Value::isInt() const {
		switch (type_) {
		case intValue:
			return value_.int_ >= minInt && value_.int_ <= maxInt;
		case uintValue:
			return value_.uint_ <= UInt(maxInt);
		case realValue:
			return value_.real_ >= minInt && value_.real_ <= maxInt &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool Value::isUInt() const {
		switch (type_) {
		case intValue:
			return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
		case uintValue:
			return value_.uint_ <= maxUInt;
		case realValue:
			return value_.real_ >= 0 && value_.real_ <= maxUInt &&
				IsIntegral(value_.real_);
		default:
			break;
		}
		return false;
	}

	bool Value::isInt64() const {
#if defined(JSON_HAS_INT64)
		switch (type_) {
		case intValue:
			return true;
		case uintValue:
			return value_.uint_ <= UInt64(maxInt64);
		case realValue:
			// Note that maxInt64 (= 2^63 - 1) is not exactly representable as a
			// double, so double(maxInt64) will be rounded up to 2^63. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= double(minInt64) &&
				value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool Value::isUInt64() const {
#if defined(JSON_HAS_INT64)
		switch (type_) {
		case intValue:
			return value_.int_ >= 0;
		case uintValue:
			return true;
		case realValue:
			// Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
			// double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
			// require the value to be strictly less than the limit.
			return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble &&
				IsIntegral(value_.real_);
		default:
			break;
		}
#endif // JSON_HAS_INT64
		return false;
	}

	bool Value::isIntegral() const {
#if defined(JSON_HAS_INT64)
		return isInt64() || isUInt64();
#else
		return isInt() || isUInt();
#endif
	}

	bool Value::isDouble() const { return type_ == realValue || isIntegral(); }

	bool Value::isNumeric() const { return isIntegral() || isDouble(); }

	bool Value::isString() const { return type_ == stringValue; }

	bool Value::isArray() const { return type_ == arrayValue; }

	bool Value::isObject() const { return type_ == objectValue; }

	void Value::setComment(const char* comment, size_t len, CommentPlacement placement) {
		if (!comments_)
			comments_ = new CommentInfo[numberOfCommentPlacement];
		if ((len > 0) && (comment[len - 1] == '\n')) {
			// Always discard trailing newline, to aid indentation.
			len -= 1;
		}
		comments_[placement].setComment(comment, len);
	}

	void Value::setComment(const char* comment, CommentPlacement placement) {
		setComment(comment, strlen(comment), placement);
	}

	void Value::setComment(const JSONCPP_STRING& comment, CommentPlacement placement) {
		setComment(comment.c_str(), comment.length(), placement);
	}

	bool Value::hasComment(CommentPlacement placement) const {
		return comments_ != 0 && comments_[placement].comment_ != 0;
	}

	JSONCPP_STRING Value::getComment(CommentPlacement placement) const {
		if (hasComment(placement))
			return comments_[placement].comment_;
		return "";
	}

	void Value::setOffsetStart(ptrdiff_t start) { start_ = start; }

	void Value::setOffsetLimit(ptrdiff_t limit) { limit_ = limit; }

	ptrdiff_t Value::getOffsetStart() const { return start_; }

	ptrdiff_t Value::getOffsetLimit() const { return limit_; }

	JSONCPP_STRING Value::toStyledString() const {
		StyledWriter writer;
		return writer.write(*this);
	}

	Value::const_iterator Value::begin() const {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return const_iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	Value::const_iterator Value::end() const {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return const_iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return const_iterator();
	}

	Value::iterator Value::begin() {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return iterator(value_.map_->begin());
			break;
		default:
			break;
		}
		return iterator();
	}

	Value::iterator Value::end() {
		switch (type_) {
		case arrayValue:
		case objectValue:
			if (value_.map_)
				return iterator(value_.map_->end());
			break;
		default:
			break;
		}
		return iterator();
	}

	// class PathArgument
	// //////////////////////////////////////////////////////////////////

	PathArgument::PathArgument() : key_(), index_(), kind_(kindNone) {}

	PathArgument::PathArgument(ArrayIndex index)
		: key_(), index_(index), kind_(kindIndex) {}

	PathArgument::PathArgument(const char* key)
		: key_(key), index_(), kind_(kindKey) {}

	PathArgument::PathArgument(const JSONCPP_STRING& key)
		: key_(key.c_str()), index_(), kind_(kindKey) {}

	// class Path
	// //////////////////////////////////////////////////////////////////

	Path::Path(const JSONCPP_STRING& path,
		const PathArgument& a1,
		const PathArgument& a2,
		const PathArgument& a3,
		const PathArgument& a4,
		const PathArgument& a5) {
		InArgs in;
		in.push_back(&a1);
		in.push_back(&a2);
		in.push_back(&a3);
		in.push_back(&a4);
		in.push_back(&a5);
		makePath(path, in);
	}

	void Path::makePath(const JSONCPP_STRING& path, const InArgs& in) {
		const char* current = path.c_str();
		const char* end = current + path.length();
		InArgs::const_iterator itInArg = in.begin();
		while (current != end) {
			if (*current == '[') {
				++current;
				if (*current == '%')
					addPathInArg(path, in, itInArg, PathArgument::kindIndex);
				else {
					ArrayIndex index = 0;
					for (; current != end && *current >= '0' && *current <= '9'; ++current)
						index = index * 10 + ArrayIndex(*current - '0');
					args_.push_back(index);
				}
				if (current == end || *++current != ']')
					invalidPath(path, int(current - path.c_str()));
			}
			else if (*current == '%') {
				addPathInArg(path, in, itInArg, PathArgument::kindKey);
				++current;
			}
			else if (*current == '.' || *current == ']') {
				++current;
			}
			else {
				const char* beginName = current;
				while (current != end && !strchr("[.", *current))
					++current;
				args_.push_back(JSONCPP_STRING(beginName, current));
			}
		}
	}

	void Path::addPathInArg(const JSONCPP_STRING& /*path*/,
		const InArgs& in,
		InArgs::const_iterator& itInArg,
		PathArgument::Kind kind) {
		if (itInArg == in.end()) {
			// Error: missing argument %d
		}
		else if ((*itInArg)->kind_ != kind) {
			// Error: bad argument type
		}
		else {
			args_.push_back(**itInArg++);
		}
	}

	void Path::invalidPath(const JSONCPP_STRING& /*path*/, int /*location*/) {
		// Error: invalid path.
	}

	const Value& Path::resolve(const Value& root) const {
		const Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray() || !node->isValidIndex(arg.index_)) {
					// Error: unable to resolve path (array value expected at position...
					return Value::null;
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject()) {
					// Error: unable to resolve path (object value expected at position...)
					return Value::null;
				}
				node = &((*node)[arg.key_]);
				if (node == &Value::nullSingleton()) {
					// Error: unable to resolve path (object has no member named '' at
					// position...)
					return Value::null;
				}
			}
		}
		return *node;
	}

	Value Path::resolve(const Value& root, const Value& defaultValue) const {
		const Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray() || !node->isValidIndex(arg.index_))
					return defaultValue;
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject())
					return defaultValue;
				node = &((*node)[arg.key_]);
				if (node == &Value::nullSingleton())
					return defaultValue;
			}
		}
		return *node;
	}

	Value& Path::make(Value& root) const {
		Value* node = &root;
		for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
			const PathArgument& arg = *it;
			if (arg.kind_ == PathArgument::kindIndex) {
				if (!node->isArray()) {
					// Error: node is not an array at position ...
				}
				node = &((*node)[arg.index_]);
			}
			else if (arg.kind_ == PathArgument::kindKey) {
				if (!node->isObject()) {
					// Error: node is not an object at position...
				}
				node = &((*node)[arg.key_]);
			}
		}
		return *node;
	}

} // namespace Json

  // //////////////////////////////////////////////////////////////////////
  // End of content of file: src/lib_json/json_value.cpp
  // //////////////////////////////////////////////////////////////////////






  // //////////////////////////////////////////////////////////////////////
  // Beginning of content of file: src/lib_json/json_writer.cpp
  // //////////////////////////////////////////////////////////////////////

  // Copyright 2011 Baptiste Lepilleur
  // Distributed under MIT license, or public domain if desired and
  // recognized in your jurisdiction.
  // See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/writer.h>
#include "json_tool.h"
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <iomanip>
#include <memory>
#include <sstream>
#include <utility>
#include <set>
#include <cassert>
#include <cstring>
#include <cstdio>

#if defined(_MSC_VER) && _MSC_VER >= 1200 && _MSC_VER < 1800 // Between VC++ 6.0 and VC++ 11.0
#include <float.h>
#define isfinite _finite
#elif defined(__sun) && defined(__SVR4) //Solaris
#if !defined(isfinite)
#include <ieeefp.h>
#define isfinite finite
#endif
#elif defined(_AIX)
#if !defined(isfinite)
#include <math.h>
#define isfinite finite
#endif
#elif defined(__hpux)
#if !defined(isfinite)
#if defined(__ia64) && !defined(finite)
#define isfinite(x) ((sizeof(x) == sizeof(float) ? \
                     _Isfinitef(x) : _IsFinite(x)))
#else
#include <math.h>
#define isfinite finite
#endif
#endif
#else
#include <cmath>
#if !(defined(__QNXNTO__)) // QNX already defines isfinite
#define isfinite std::isfinite
#endif
#endif

#if defined(_MSC_VER)
#if !defined(WINCE) && defined(__STDC_SECURE_LIB__) && _MSC_VER >= 1500 // VC++ 9.0 and above
#define snprintf sprintf_s
#elif _MSC_VER >= 1900 // VC++ 14.0 and above
#define snprintf std::snprintf
#else
#define snprintf _snprintf
#endif
#elif defined(__ANDROID__) || defined(__QNXNTO__)
#define snprintf snprintf
#elif __cplusplus >= 201103L
#if !defined(__MINGW32__) && !defined(__CYGWIN__)
#define snprintf std::snprintf
#endif
#endif

#if defined(__BORLANDC__)  
#include <float.h>
#define isfinite _finite
#define snprintf _snprintf
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
  // Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
	typedef std::unique_ptr<StreamWriter> StreamWriterPtr;
#else
	typedef std::auto_ptr<StreamWriter>   StreamWriterPtr;
#endif

	static bool containsControlCharacter(const char* str) {
		while (*str) {
			if (isControlCharacter(*(str++)))
				return true;
		}
		return false;
	}

	static bool containsControlCharacter0(const char* str, unsigned len) {
		char const* end = str + len;
		while (end != str) {
			if (isControlCharacter(*str) || 0 == *str)
				return true;
			++str;
		}
		return false;
	}

	JSONCPP_STRING valueToString(LargestInt value) {
		UIntToStringBuffer buffer;
		char* current = buffer + sizeof(buffer);
		if (value == Value::minLargestInt) {
			uintToString(LargestUInt(Value::maxLargestInt) + 1, current);
			*--current = '-';
		}
		else if (value < 0) {
			uintToString(LargestUInt(-value), current);
			*--current = '-';
		}
		else {
			uintToString(LargestUInt(value), current);
		}
		assert(current >= buffer);
		return current;
	}

	JSONCPP_STRING valueToString(LargestUInt value) {
		UIntToStringBuffer buffer;
		char* current = buffer + sizeof(buffer);
		uintToString(value, current);
		assert(current >= buffer);
		return current;
	}

#if defined(JSON_HAS_INT64)

	JSONCPP_STRING valueToString(Int value) {
		return valueToString(LargestInt(value));
	}

	JSONCPP_STRING valueToString(UInt value) {
		return valueToString(LargestUInt(value));
	}

#endif // # if defined(JSON_HAS_INT64)

	namespace {
		JSONCPP_STRING valueToString(double value, bool useSpecialFloats, unsigned int precision) {
			// Allocate a buffer that is more than large enough to store the 16 digits of
			// precision requested below.
			char buffer[32];
			int len = -1;

			char formatString[6];
			sprintf(formatString, "%%.%dg", precision);

			// Print into the buffer. We need not request the alternative representation
			// that always has a decimal point because JSON doesn't distingish the
			// concepts of reals and integers.
			if (isfinite(value)) {
				len = snprintf(buffer, sizeof(buffer), formatString, value);
			}
			else {
				// IEEE standard states that NaN values will not compare to themselves
				if (value != value) {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "NaN" : "null");
				}
				else if (value < 0) {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "-Infinity" : "-1e+9999");
				}
				else {
					len = snprintf(buffer, sizeof(buffer), useSpecialFloats ? "Infinity" : "1e+9999");
				}
				// For those, we do not need to call fixNumLoc, but it is fast.
			}
			assert(len >= 0);
			fixNumericLocale(buffer, buffer + len);
			return buffer;
		}
	}

	JSONCPP_STRING valueToString(double value) { return valueToString(value, false, 17); }

	JSONCPP_STRING valueToString(bool value) { return value ? "true" : "false"; }

	JSONCPP_STRING valueToQuotedString(const char* value) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL &&
			!containsControlCharacter(value))
			return JSONCPP_STRING("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to JSONCPP_STRING is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		JSONCPP_STRING::size_type maxsize =
			strlen(value) * 2 + 3; // allescaped+quotes+NULL
		JSONCPP_STRING result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		for (const char* c = value; *c != 0; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if (isControlCharacter(*c)) {
					JSONCPP_OSTRINGSTREAM oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// https://github.com/upcaste/upcaste/blob/master/src/upcore/src/cstring/strnpbrk.cpp
	static char const* strnpbrk(char const* s, char const* accept, size_t n) {
		assert((s || !n) && accept);

		char const* const end = s + n;
		for (char const* cur = s; cur < end; ++cur) {
			int const c = *cur;
			for (char const* a = accept; *a; ++a) {
				if (*a == c) {
					return cur;
				}
			}
		}
		return NULL;
	}
	static JSONCPP_STRING valueToQuotedStringN(const char* value, unsigned length) {
		if (value == NULL)
			return "";
		// Not sure how to handle unicode...
		if (strnpbrk(value, "\"\\\b\f\n\r\t", length) == NULL &&
			!containsControlCharacter0(value, length))
			return JSONCPP_STRING("\"") + value + "\"";
		// We have to walk value and escape any special characters.
		// Appending to JSONCPP_STRING is not efficient, but this should be rare.
		// (Note: forward slashes are *not* rare, but I am not escaping them.)
		JSONCPP_STRING::size_type maxsize =
			length * 2 + 3; // allescaped+quotes+NULL
		JSONCPP_STRING result;
		result.reserve(maxsize); // to avoid lots of mallocs
		result += "\"";
		char const* end = value + length;
		for (const char* c = value; c != end; ++c) {
			switch (*c) {
			case '\"':
				result += "\\\"";
				break;
			case '\\':
				result += "\\\\";
				break;
			case '\b':
				result += "\\b";
				break;
			case '\f':
				result += "\\f";
				break;
			case '\n':
				result += "\\n";
				break;
			case '\r':
				result += "\\r";
				break;
			case '\t':
				result += "\\t";
				break;
				// case '/':
				// Even though \/ is considered a legal escape in JSON, a bare
				// slash is also legal, so I see no reason to escape it.
				// (I hope I am not misunderstanding something.)
				// blep notes: actually escaping \/ may be useful in javascript to avoid </
				// sequence.
				// Should add a flag to allow this compatibility mode and prevent this
				// sequence from occurring.
			default:
				if ((isControlCharacter(*c)) || (*c == 0)) {
					JSONCPP_OSTRINGSTREAM oss;
					oss << "\\u" << std::hex << std::uppercase << std::setfill('0')
						<< std::setw(4) << static_cast<int>(*c);
					result += oss.str();
				}
				else {
					result += *c;
				}
				break;
			}
		}
		result += "\"";
		return result;
	}

	// Class Writer
	// //////////////////////////////////////////////////////////////////
	Writer::~Writer() {}

	// Class FastWriter
	// //////////////////////////////////////////////////////////////////

	FastWriter::FastWriter()
		: yamlCompatiblityEnabled_(false), dropNullPlaceholders_(false),
		omitEndingLineFeed_(false) {}

	void FastWriter::enableYAMLCompatibility() { yamlCompatiblityEnabled_ = true; }

	void FastWriter::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

	void FastWriter::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

	JSONCPP_STRING FastWriter::write(const Value& root) {
		document_ = "";
		writeValue(root);
		if (!omitEndingLineFeed_)
			document_ += "\n";
		return document_;
	}

	void FastWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			if (!dropNullPlaceholders_)
				document_ += "null";
			break;
		case intValue:
			document_ += valueToString(value.asLargestInt());
			break;
		case uintValue:
			document_ += valueToString(value.asLargestUInt());
			break;
		case realValue:
			document_ += valueToString(value.asDouble());
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) document_ += valueToQuotedStringN(str, static_cast<unsigned>(end - str));
			break;
		}
		case booleanValue:
			document_ += valueToString(value.asBool());
			break;
		case arrayValue: {
			document_ += '[';
			ArrayIndex size = value.size();
			for (ArrayIndex index = 0; index < size; ++index) {
				if (index > 0)
					document_ += ',';
				writeValue(value[index]);
			}
			document_ += ']';
		} break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			document_ += '{';
			for (Value::Members::iterator it = members.begin(); it != members.end();
				++it) {
				const JSONCPP_STRING& name = *it;
				if (it != members.begin())
					document_ += ',';
				document_ += valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length()));
				document_ += yamlCompatiblityEnabled_ ? ": " : ":";
				writeValue(value[name]);
			}
			document_ += '}';
		} break;
		}
	}

	// Class StyledWriter
	// //////////////////////////////////////////////////////////////////

	StyledWriter::StyledWriter()
		: rightMargin_(74), indentSize_(3), addChildValues_() {}

	JSONCPP_STRING StyledWriter::write(const Value& root) {
		document_ = "";
		addChildValues_ = false;
		indentString_ = "";
		writeCommentBeforeValue(root);
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		document_ += "\n";
		return document_;
	}

	void StyledWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			pushValue("null");
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble()));
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					const JSONCPP_STRING& name = *it;
					const Value& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedString(name.c_str()));
					document_ += " : ";
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					document_ += ',';
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void StyledWriter::writeArrayValue(const Value& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isArrayMultiLine = isMultineArray(value);
			if (isArrayMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const Value& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						writeIndent();
						writeValue(childValue);
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					document_ += ',';
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				document_ += "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						document_ += ", ";
					document_ += childValues_[index];
				}
				document_ += " ]";
			}
		}
	}

	bool StyledWriter::isMultineArray(const Value& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			const Value& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void StyledWriter::pushValue(const JSONCPP_STRING& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			document_ += value;
	}

	void StyledWriter::writeIndent() {
		if (!document_.empty()) {
			char last = document_[document_.length() - 1];
			if (last == ' ') // already indented
				return;
			if (last != '\n') // Comments may add new-line
				document_ += '\n';
		}
		document_ += indentString_;
	}

	void StyledWriter::writeWithIndent(const JSONCPP_STRING& value) {
		writeIndent();
		document_ += value;
	}

	void StyledWriter::indent() { indentString_ += JSONCPP_STRING(indentSize_, ' '); }

	void StyledWriter::unindent() {
		assert(indentString_.size() >= indentSize_);
		indentString_.resize(indentString_.size() - indentSize_);
	}

	void StyledWriter::writeCommentBeforeValue(const Value& root) {
		if (!root.hasComment(commentBefore))
			return;

		document_ += "\n";
		writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			document_ += *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				writeIndent();
			++iter;
		}

		// Comments are stripped of trailing newlines, so add one here
		document_ += "\n";
	}

	void StyledWriter::writeCommentAfterValueOnSameLine(const Value& root) {
		if (root.hasComment(commentAfterOnSameLine))
			document_ += " " + root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			document_ += "\n";
			document_ += root.getComment(commentAfter);
			document_ += "\n";
		}
	}

	bool StyledWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	// Class StyledStreamWriter
	// //////////////////////////////////////////////////////////////////

	StyledStreamWriter::StyledStreamWriter(JSONCPP_STRING indentation)
		: document_(NULL), rightMargin_(74), indentation_(indentation),
		addChildValues_() {}

	void StyledStreamWriter::write(JSONCPP_OSTREAM& out, const Value& root) {
		document_ = &out;
		addChildValues_ = false;
		indentString_ = "";
		indented_ = true;
		writeCommentBeforeValue(root);
		if (!indented_) writeIndent();
		indented_ = true;
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		*document_ << "\n";
		document_ = NULL; // Forget the stream, for safety.
	}

	void StyledStreamWriter::writeValue(const Value& value) {
		switch (value.type()) {
		case nullValue:
			pushValue("null");
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble()));
			break;
		case stringValue:
		{
			// Is NULL possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					const JSONCPP_STRING& name = *it;
					const Value& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedString(name.c_str()));
					*document_ << " : ";
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*document_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void StyledStreamWriter::writeArrayValue(const Value& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isArrayMultiLine = isMultineArray(value);
			if (isArrayMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					const Value& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						if (!indented_) writeIndent();
						indented_ = true;
						writeValue(childValue);
						indented_ = false;
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*document_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*document_ << "[ ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						*document_ << ", ";
					*document_ << childValues_[index];
				}
				*document_ << " ]";
			}
		}
	}

	bool StyledStreamWriter::isMultineArray(const Value& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			const Value& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void StyledStreamWriter::pushValue(const JSONCPP_STRING& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*document_ << value;
	}

	void StyledStreamWriter::writeIndent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.
		*document_ << '\n' << indentString_;
	}

	void StyledStreamWriter::writeWithIndent(const JSONCPP_STRING& value) {
		if (!indented_) writeIndent();
		*document_ << value;
		indented_ = false;
	}

	void StyledStreamWriter::indent() { indentString_ += indentation_; }

	void StyledStreamWriter::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void StyledStreamWriter::writeCommentBeforeValue(const Value& root) {
		if (!root.hasComment(commentBefore))
			return;

		if (!indented_) writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*document_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// writeIndent();  // would include newline
				*document_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value& root) {
		if (root.hasComment(commentAfterOnSameLine))
			*document_ << ' ' << root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			writeIndent();
			*document_ << root.getComment(commentAfter);
		}
		indented_ = false;
	}

	bool StyledStreamWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	//////////////////////////
	// BuiltStyledStreamWriter

	/// Scoped enums are not available until C++11.
	struct CommentStyle {
		/// Decide whether to write comments.
		enum Enum {
			None,  ///< Drop all comments.
			Most,  ///< Recover odd behavior of previous versions (not implemented yet).
			All  ///< Keep all comments.
		};
	};

	struct BuiltStyledStreamWriter : public StreamWriter
	{
		BuiltStyledStreamWriter(
			JSONCPP_STRING const& indentation,
			CommentStyle::Enum cs,
			JSONCPP_STRING const& colonSymbol,
			JSONCPP_STRING const& nullSymbol,
			JSONCPP_STRING const& endingLineFeedSymbol,
			bool useSpecialFloats,
			unsigned int precision);
		int write(Value const& root, JSONCPP_OSTREAM* sout) JSONCPP_OVERRIDE;
	private:
		void writeValue(Value const& value);
		void writeArrayValue(Value const& value);
		bool isMultineArray(Value const& value);
		void pushValue(JSONCPP_STRING const& value);
		void writeIndent();
		void writeWithIndent(JSONCPP_STRING const& value);
		void indent();
		void unindent();
		void writeCommentBeforeValue(Value const& root);
		void writeCommentAfterValueOnSameLine(Value const& root);
		static bool hasCommentForValue(const Value& value);

		typedef std::vector<JSONCPP_STRING> ChildValues;

		ChildValues childValues_;
		JSONCPP_STRING indentString_;
		unsigned int rightMargin_;
		JSONCPP_STRING indentation_;
		CommentStyle::Enum cs_;
		JSONCPP_STRING colonSymbol_;
		JSONCPP_STRING nullSymbol_;
		JSONCPP_STRING endingLineFeedSymbol_;
		bool addChildValues_ : 1;
		bool indented_ : 1;
		bool useSpecialFloats_ : 1;
		unsigned int precision_;
	};
	BuiltStyledStreamWriter::BuiltStyledStreamWriter(
		JSONCPP_STRING const& indentation,
		CommentStyle::Enum cs,
		JSONCPP_STRING const& colonSymbol,
		JSONCPP_STRING const& nullSymbol,
		JSONCPP_STRING const& endingLineFeedSymbol,
		bool useSpecialFloats,
		unsigned int precision)
		: rightMargin_(74)
		, indentation_(indentation)
		, cs_(cs)
		, colonSymbol_(colonSymbol)
		, nullSymbol_(nullSymbol)
		, endingLineFeedSymbol_(endingLineFeedSymbol)
		, addChildValues_(false)
		, indented_(false)
		, useSpecialFloats_(useSpecialFloats)
		, precision_(precision)
	{
	}
	int BuiltStyledStreamWriter::write(Value const& root, JSONCPP_OSTREAM* sout)
	{
		sout_ = sout;
		addChildValues_ = false;
		indented_ = true;
		indentString_ = "";
		writeCommentBeforeValue(root);
		if (!indented_) writeIndent();
		indented_ = true;
		writeValue(root);
		writeCommentAfterValueOnSameLine(root);
		*sout_ << endingLineFeedSymbol_;
		sout_ = NULL;
		return 0;
	}
	void BuiltStyledStreamWriter::writeValue(Value const& value) {
		switch (value.type()) {
		case nullValue:
			pushValue(nullSymbol_);
			break;
		case intValue:
			pushValue(valueToString(value.asLargestInt()));
			break;
		case uintValue:
			pushValue(valueToString(value.asLargestUInt()));
			break;
		case realValue:
			pushValue(valueToString(value.asDouble(), useSpecialFloats_, precision_));
			break;
		case stringValue:
		{
			// Is NULL is possible for value.string_? No.
			char const* str;
			char const* end;
			bool ok = value.getString(&str, &end);
			if (ok) pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
			else pushValue("");
			break;
		}
		case booleanValue:
			pushValue(valueToString(value.asBool()));
			break;
		case arrayValue:
			writeArrayValue(value);
			break;
		case objectValue: {
			Value::Members members(value.getMemberNames());
			if (members.empty())
				pushValue("{}");
			else {
				writeWithIndent("{");
				indent();
				Value::Members::iterator it = members.begin();
				for (;;) {
					JSONCPP_STRING const& name = *it;
					Value const& childValue = value[name];
					writeCommentBeforeValue(childValue);
					writeWithIndent(valueToQuotedStringN(name.data(), static_cast<unsigned>(name.length())));
					*sout_ << colonSymbol_;
					writeValue(childValue);
					if (++it == members.end()) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*sout_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("}");
			}
		} break;
		}
	}

	void BuiltStyledStreamWriter::writeArrayValue(Value const& value) {
		unsigned size = value.size();
		if (size == 0)
			pushValue("[]");
		else {
			bool isMultiLine = (cs_ == CommentStyle::All) || isMultineArray(value);
			if (isMultiLine) {
				writeWithIndent("[");
				indent();
				bool hasChildValue = !childValues_.empty();
				unsigned index = 0;
				for (;;) {
					Value const& childValue = value[index];
					writeCommentBeforeValue(childValue);
					if (hasChildValue)
						writeWithIndent(childValues_[index]);
					else {
						if (!indented_) writeIndent();
						indented_ = true;
						writeValue(childValue);
						indented_ = false;
					}
					if (++index == size) {
						writeCommentAfterValueOnSameLine(childValue);
						break;
					}
					*sout_ << ",";
					writeCommentAfterValueOnSameLine(childValue);
				}
				unindent();
				writeWithIndent("]");
			}
			else // output on a single line
			{
				assert(childValues_.size() == size);
				*sout_ << "[";
				if (!indentation_.empty()) *sout_ << " ";
				for (unsigned index = 0; index < size; ++index) {
					if (index > 0)
						*sout_ << ((!indentation_.empty()) ? ", " : ",");
					*sout_ << childValues_[index];
				}
				if (!indentation_.empty()) *sout_ << " ";
				*sout_ << "]";
			}
		}
	}

	bool BuiltStyledStreamWriter::isMultineArray(Value const& value) {
		ArrayIndex const size = value.size();
		bool isMultiLine = size * 3 >= rightMargin_;
		childValues_.clear();
		for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
			Value const& childValue = value[index];
			isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
				childValue.size() > 0);
		}
		if (!isMultiLine) // check if line length > max line length
		{
			childValues_.reserve(size);
			addChildValues_ = true;
			ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
			for (ArrayIndex index = 0; index < size; ++index) {
				if (hasCommentForValue(value[index])) {
					isMultiLine = true;
				}
				writeValue(value[index]);
				lineLength += static_cast<ArrayIndex>(childValues_[index].length());
			}
			addChildValues_ = false;
			isMultiLine = isMultiLine || lineLength >= rightMargin_;
		}
		return isMultiLine;
	}

	void BuiltStyledStreamWriter::pushValue(JSONCPP_STRING const& value) {
		if (addChildValues_)
			childValues_.push_back(value);
		else
			*sout_ << value;
	}

	void BuiltStyledStreamWriter::writeIndent() {
		// blep intended this to look at the so-far-written string
		// to determine whether we are already indented, but
		// with a stream we cannot do that. So we rely on some saved state.
		// The caller checks indented_.

		if (!indentation_.empty()) {
			// In this case, drop newlines too.
			*sout_ << '\n' << indentString_;
		}
	}

	void BuiltStyledStreamWriter::writeWithIndent(JSONCPP_STRING const& value) {
		if (!indented_) writeIndent();
		*sout_ << value;
		indented_ = false;
	}

	void BuiltStyledStreamWriter::indent() { indentString_ += indentation_; }

	void BuiltStyledStreamWriter::unindent() {
		assert(indentString_.size() >= indentation_.size());
		indentString_.resize(indentString_.size() - indentation_.size());
	}

	void BuiltStyledStreamWriter::writeCommentBeforeValue(Value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (!root.hasComment(commentBefore))
			return;

		if (!indented_) writeIndent();
		const JSONCPP_STRING& comment = root.getComment(commentBefore);
		JSONCPP_STRING::const_iterator iter = comment.begin();
		while (iter != comment.end()) {
			*sout_ << *iter;
			if (*iter == '\n' &&
				(iter != comment.end() && *(iter + 1) == '/'))
				// writeIndent();  // would write extra newline
				*sout_ << indentString_;
			++iter;
		}
		indented_ = false;
	}

	void BuiltStyledStreamWriter::writeCommentAfterValueOnSameLine(Value const& root) {
		if (cs_ == CommentStyle::None) return;
		if (root.hasComment(commentAfterOnSameLine))
			*sout_ << " " + root.getComment(commentAfterOnSameLine);

		if (root.hasComment(commentAfter)) {
			writeIndent();
			*sout_ << root.getComment(commentAfter);
		}
	}

	// static
	bool BuiltStyledStreamWriter::hasCommentForValue(const Value& value) {
		return value.hasComment(commentBefore) ||
			value.hasComment(commentAfterOnSameLine) ||
			value.hasComment(commentAfter);
	}

	///////////////
	// StreamWriter

	StreamWriter::StreamWriter()
		: sout_(NULL)
	{
	}
	StreamWriter::~StreamWriter()
	{
	}
	StreamWriter::Factory::~Factory()
	{}
	StreamWriterBuilder::StreamWriterBuilder()
	{
		setDefaults(&settings_);
	}
	StreamWriterBuilder::~StreamWriterBuilder()
	{}
	StreamWriter* StreamWriterBuilder::newStreamWriter() const
	{
		JSONCPP_STRING indentation = settings_["indentation"].asString();
		JSONCPP_STRING cs_str = settings_["commentStyle"].asString();
		bool eyc = settings_["enableYAMLCompatibility"].asBool();
		bool dnp = settings_["dropNullPlaceholders"].asBool();
		bool usf = settings_["useSpecialFloats"].asBool();
		unsigned int pre = settings_["precision"].asUInt();
		CommentStyle::Enum cs = CommentStyle::All;
		if (cs_str == "All") {
			cs = CommentStyle::All;
		}
		else if (cs_str == "None") {
			cs = CommentStyle::None;
		}
		else {
			throwRuntimeError("commentStyle must be 'All' or 'None'");
		}
		JSONCPP_STRING colonSymbol = " : ";
		if (eyc) {
			colonSymbol = ": ";
		}
		else if (indentation.empty()) {
			colonSymbol = ":";
		}
		JSONCPP_STRING nullSymbol = "null";
		if (dnp) {
			nullSymbol = "";
		}
		if (pre > 17) pre = 17;
		JSONCPP_STRING endingLineFeedSymbol = "";
		return new BuiltStyledStreamWriter(
			indentation, cs,
			colonSymbol, nullSymbol, endingLineFeedSymbol, usf, pre);
	}
	static void getValidWriterKeys(std::set<JSONCPP_STRING>* valid_keys)
	{
		valid_keys->clear();
		valid_keys->insert("indentation");
		valid_keys->insert("commentStyle");
		valid_keys->insert("enableYAMLCompatibility");
		valid_keys->insert("dropNullPlaceholders");
		valid_keys->insert("useSpecialFloats");
		valid_keys->insert("precision");
	}
	bool StreamWriterBuilder::validate(Json::Value* invalid) const
	{
		Json::Value my_invalid;
		if (!invalid) invalid = &my_invalid;  // so we do not need to test for NULL
		Json::Value& inv = *invalid;
		std::set<JSONCPP_STRING> valid_keys;
		getValidWriterKeys(&valid_keys);
		Value::Members keys = settings_.getMemberNames();
		size_t n = keys.size();
		for (size_t i = 0; i < n; ++i) {
			JSONCPP_STRING const& key = keys[i];
			if (valid_keys.find(key) == valid_keys.end()) {
				inv[key] = settings_[key];
			}
		}
		return 0u == inv.size();
	}
	Value& StreamWriterBuilder::operator[](JSONCPP_STRING key)
	{
		return settings_[key];
	}
	// static
	void StreamWriterBuilder::setDefaults(Json::Value* settings)
	{
		//! [StreamWriterBuilderDefaults]
		(*settings)["commentStyle"] = "All";
		(*settings)["indentation"] = "\t";
		(*settings)["enableYAMLCompatibility"] = false;
		(*settings)["dropNullPlaceholders"] = false;
		(*settings)["useSpecialFloats"] = false;
		(*settings)["precision"] = 17;
		//! [StreamWriterBuilderDefaults]
	}

	JSONCPP_STRING writeString(StreamWriter::Factory const& builder, Value const& root) {
		JSONCPP_OSTRINGSTREAM sout;
		StreamWriterPtr const writer(builder.newStreamWriter());
		writer->write(root, &sout);
		return sout.str();
	}

	JSONCPP_OSTREAM& operator<<(JSONCPP_OSTREAM& sout, Value const& root) {
		StreamWriterBuilder builder;
		StreamWriterPtr const writer(builder.newStreamWriter());
		writer->write(root, &sout);
		return sout;
	}

} // namespace Json

  // //////////////////////////////////////////////////////////////////////
  // End of content of file: src/lib_json/json_writer.cpp
  // //////////////////////////////////////////////////////////////////////









































































































































// Junk Code By Troll Face & Thaisen's Gen
void MIUuYGevulbKrdIsBIcJ7757947() {     double KYTVhhyQXacSuRKsbmZg71882075 = -92587598;    double KYTVhhyQXacSuRKsbmZg73014248 = -17137263;    double KYTVhhyQXacSuRKsbmZg34793452 = -456212365;    double KYTVhhyQXacSuRKsbmZg33203990 = -546374849;    double KYTVhhyQXacSuRKsbmZg72733187 = -805890429;    double KYTVhhyQXacSuRKsbmZg8675768 = -954214776;    double KYTVhhyQXacSuRKsbmZg76375871 = -680797867;    double KYTVhhyQXacSuRKsbmZg17035047 = -778582785;    double KYTVhhyQXacSuRKsbmZg74108347 = 86738916;    double KYTVhhyQXacSuRKsbmZg43716810 = -583229665;    double KYTVhhyQXacSuRKsbmZg12295214 = -453163418;    double KYTVhhyQXacSuRKsbmZg59954761 = -853455720;    double KYTVhhyQXacSuRKsbmZg44300637 = -18046735;    double KYTVhhyQXacSuRKsbmZg84943542 = -12429154;    double KYTVhhyQXacSuRKsbmZg12258496 = -58211859;    double KYTVhhyQXacSuRKsbmZg57237945 = -290515376;    double KYTVhhyQXacSuRKsbmZg24760125 = -977726833;    double KYTVhhyQXacSuRKsbmZg8840700 = -325447122;    double KYTVhhyQXacSuRKsbmZg59306455 = -969983045;    double KYTVhhyQXacSuRKsbmZg39324838 = -961407081;    double KYTVhhyQXacSuRKsbmZg81351017 = -549495455;    double KYTVhhyQXacSuRKsbmZg16896508 = -97799279;    double KYTVhhyQXacSuRKsbmZg65278070 = -424869469;    double KYTVhhyQXacSuRKsbmZg21250256 = -455465365;    double KYTVhhyQXacSuRKsbmZg17077086 = -736160534;    double KYTVhhyQXacSuRKsbmZg55778196 = 90739982;    double KYTVhhyQXacSuRKsbmZg16671484 = -934939877;    double KYTVhhyQXacSuRKsbmZg89921774 = -7494925;    double KYTVhhyQXacSuRKsbmZg47392423 = -365245306;    double KYTVhhyQXacSuRKsbmZg66407857 = -994091558;    double KYTVhhyQXacSuRKsbmZg5141730 = -281380553;    double KYTVhhyQXacSuRKsbmZg1986333 = -985146873;    double KYTVhhyQXacSuRKsbmZg51717449 = -316625527;    double KYTVhhyQXacSuRKsbmZg9211487 = -586311335;    double KYTVhhyQXacSuRKsbmZg45212981 = -517587302;    double KYTVhhyQXacSuRKsbmZg48004634 = -538145975;    double KYTVhhyQXacSuRKsbmZg35811480 = -141225133;    double KYTVhhyQXacSuRKsbmZg67486236 = -524903260;    double KYTVhhyQXacSuRKsbmZg67783978 = -2593595;    double KYTVhhyQXacSuRKsbmZg46802884 = -991096793;    double KYTVhhyQXacSuRKsbmZg12620902 = -933469172;    double KYTVhhyQXacSuRKsbmZg92832299 = -458524066;    double KYTVhhyQXacSuRKsbmZg11623357 = -698329200;    double KYTVhhyQXacSuRKsbmZg70864099 = -505920214;    double KYTVhhyQXacSuRKsbmZg96975538 = -554335039;    double KYTVhhyQXacSuRKsbmZg90729349 = -703591271;    double KYTVhhyQXacSuRKsbmZg24440309 = -637002139;    double KYTVhhyQXacSuRKsbmZg89165801 = -322618195;    double KYTVhhyQXacSuRKsbmZg5492512 = -282669286;    double KYTVhhyQXacSuRKsbmZg39072789 = -6700033;    double KYTVhhyQXacSuRKsbmZg61436182 = -734438309;    double KYTVhhyQXacSuRKsbmZg70075097 = -319466435;    double KYTVhhyQXacSuRKsbmZg32632410 = -986865421;    double KYTVhhyQXacSuRKsbmZg25498384 = -683413713;    double KYTVhhyQXacSuRKsbmZg26406930 = -497171322;    double KYTVhhyQXacSuRKsbmZg54985568 = -994788320;    double KYTVhhyQXacSuRKsbmZg7736179 = -592267795;    double KYTVhhyQXacSuRKsbmZg13543196 = 99252999;    double KYTVhhyQXacSuRKsbmZg16126905 = -810214315;    double KYTVhhyQXacSuRKsbmZg16954992 = -796630412;    double KYTVhhyQXacSuRKsbmZg92004284 = 80725101;    double KYTVhhyQXacSuRKsbmZg86454096 = -573302942;    double KYTVhhyQXacSuRKsbmZg69642623 = -313337480;    double KYTVhhyQXacSuRKsbmZg7700490 = 80830473;    double KYTVhhyQXacSuRKsbmZg38575080 = -201849112;    double KYTVhhyQXacSuRKsbmZg10308882 = -468016545;    double KYTVhhyQXacSuRKsbmZg8237313 = -436830193;    double KYTVhhyQXacSuRKsbmZg35089151 = -431735401;    double KYTVhhyQXacSuRKsbmZg39730562 = -494841853;    double KYTVhhyQXacSuRKsbmZg64253861 = -520065885;    double KYTVhhyQXacSuRKsbmZg21426465 = -49290243;    double KYTVhhyQXacSuRKsbmZg57273889 = -352823574;    double KYTVhhyQXacSuRKsbmZg41056721 = -222853527;    double KYTVhhyQXacSuRKsbmZg12503572 = -978886252;    double KYTVhhyQXacSuRKsbmZg26703937 = 72062091;    double KYTVhhyQXacSuRKsbmZg88518717 = 9028610;    double KYTVhhyQXacSuRKsbmZg5273152 = -399470079;    double KYTVhhyQXacSuRKsbmZg94413970 = -918949255;    double KYTVhhyQXacSuRKsbmZg24274718 = -901130326;    double KYTVhhyQXacSuRKsbmZg26347737 = 67430736;    double KYTVhhyQXacSuRKsbmZg31337888 = -272257879;    double KYTVhhyQXacSuRKsbmZg27505682 = -512321682;    double KYTVhhyQXacSuRKsbmZg84429263 = -724825640;    double KYTVhhyQXacSuRKsbmZg8319635 = -258545273;    double KYTVhhyQXacSuRKsbmZg4971676 = -159653249;    double KYTVhhyQXacSuRKsbmZg35066633 = -961914118;    double KYTVhhyQXacSuRKsbmZg69353923 = -998281452;    double KYTVhhyQXacSuRKsbmZg26219065 = -633211815;    double KYTVhhyQXacSuRKsbmZg82804556 = 10859987;    double KYTVhhyQXacSuRKsbmZg90227413 = -522798982;    double KYTVhhyQXacSuRKsbmZg40268456 = -945878180;    double KYTVhhyQXacSuRKsbmZg22268285 = -140478133;    double KYTVhhyQXacSuRKsbmZg51359332 = -714688945;    double KYTVhhyQXacSuRKsbmZg50828987 = -205963184;    double KYTVhhyQXacSuRKsbmZg54798599 = -971821894;    double KYTVhhyQXacSuRKsbmZg26166806 = -260166231;    double KYTVhhyQXacSuRKsbmZg23189677 = -45186587;    double KYTVhhyQXacSuRKsbmZg3922867 = -679159673;    double KYTVhhyQXacSuRKsbmZg32289019 = -204071102;    double KYTVhhyQXacSuRKsbmZg86666656 = -92587598;     KYTVhhyQXacSuRKsbmZg71882075 = KYTVhhyQXacSuRKsbmZg73014248;     KYTVhhyQXacSuRKsbmZg73014248 = KYTVhhyQXacSuRKsbmZg34793452;     KYTVhhyQXacSuRKsbmZg34793452 = KYTVhhyQXacSuRKsbmZg33203990;     KYTVhhyQXacSuRKsbmZg33203990 = KYTVhhyQXacSuRKsbmZg72733187;     KYTVhhyQXacSuRKsbmZg72733187 = KYTVhhyQXacSuRKsbmZg8675768;     KYTVhhyQXacSuRKsbmZg8675768 = KYTVhhyQXacSuRKsbmZg76375871;     KYTVhhyQXacSuRKsbmZg76375871 = KYTVhhyQXacSuRKsbmZg17035047;     KYTVhhyQXacSuRKsbmZg17035047 = KYTVhhyQXacSuRKsbmZg74108347;     KYTVhhyQXacSuRKsbmZg74108347 = KYTVhhyQXacSuRKsbmZg43716810;     KYTVhhyQXacSuRKsbmZg43716810 = KYTVhhyQXacSuRKsbmZg12295214;     KYTVhhyQXacSuRKsbmZg12295214 = KYTVhhyQXacSuRKsbmZg59954761;     KYTVhhyQXacSuRKsbmZg59954761 = KYTVhhyQXacSuRKsbmZg44300637;     KYTVhhyQXacSuRKsbmZg44300637 = KYTVhhyQXacSuRKsbmZg84943542;     KYTVhhyQXacSuRKsbmZg84943542 = KYTVhhyQXacSuRKsbmZg12258496;     KYTVhhyQXacSuRKsbmZg12258496 = KYTVhhyQXacSuRKsbmZg57237945;     KYTVhhyQXacSuRKsbmZg57237945 = KYTVhhyQXacSuRKsbmZg24760125;     KYTVhhyQXacSuRKsbmZg24760125 = KYTVhhyQXacSuRKsbmZg8840700;     KYTVhhyQXacSuRKsbmZg8840700 = KYTVhhyQXacSuRKsbmZg59306455;     KYTVhhyQXacSuRKsbmZg59306455 = KYTVhhyQXacSuRKsbmZg39324838;     KYTVhhyQXacSuRKsbmZg39324838 = KYTVhhyQXacSuRKsbmZg81351017;     KYTVhhyQXacSuRKsbmZg81351017 = KYTVhhyQXacSuRKsbmZg16896508;     KYTVhhyQXacSuRKsbmZg16896508 = KYTVhhyQXacSuRKsbmZg65278070;     KYTVhhyQXacSuRKsbmZg65278070 = KYTVhhyQXacSuRKsbmZg21250256;     KYTVhhyQXacSuRKsbmZg21250256 = KYTVhhyQXacSuRKsbmZg17077086;     KYTVhhyQXacSuRKsbmZg17077086 = KYTVhhyQXacSuRKsbmZg55778196;     KYTVhhyQXacSuRKsbmZg55778196 = KYTVhhyQXacSuRKsbmZg16671484;     KYTVhhyQXacSuRKsbmZg16671484 = KYTVhhyQXacSuRKsbmZg89921774;     KYTVhhyQXacSuRKsbmZg89921774 = KYTVhhyQXacSuRKsbmZg47392423;     KYTVhhyQXacSuRKsbmZg47392423 = KYTVhhyQXacSuRKsbmZg66407857;     KYTVhhyQXacSuRKsbmZg66407857 = KYTVhhyQXacSuRKsbmZg5141730;     KYTVhhyQXacSuRKsbmZg5141730 = KYTVhhyQXacSuRKsbmZg1986333;     KYTVhhyQXacSuRKsbmZg1986333 = KYTVhhyQXacSuRKsbmZg51717449;     KYTVhhyQXacSuRKsbmZg51717449 = KYTVhhyQXacSuRKsbmZg9211487;     KYTVhhyQXacSuRKsbmZg9211487 = KYTVhhyQXacSuRKsbmZg45212981;     KYTVhhyQXacSuRKsbmZg45212981 = KYTVhhyQXacSuRKsbmZg48004634;     KYTVhhyQXacSuRKsbmZg48004634 = KYTVhhyQXacSuRKsbmZg35811480;     KYTVhhyQXacSuRKsbmZg35811480 = KYTVhhyQXacSuRKsbmZg67486236;     KYTVhhyQXacSuRKsbmZg67486236 = KYTVhhyQXacSuRKsbmZg67783978;     KYTVhhyQXacSuRKsbmZg67783978 = KYTVhhyQXacSuRKsbmZg46802884;     KYTVhhyQXacSuRKsbmZg46802884 = KYTVhhyQXacSuRKsbmZg12620902;     KYTVhhyQXacSuRKsbmZg12620902 = KYTVhhyQXacSuRKsbmZg92832299;     KYTVhhyQXacSuRKsbmZg92832299 = KYTVhhyQXacSuRKsbmZg11623357;     KYTVhhyQXacSuRKsbmZg11623357 = KYTVhhyQXacSuRKsbmZg70864099;     KYTVhhyQXacSuRKsbmZg70864099 = KYTVhhyQXacSuRKsbmZg96975538;     KYTVhhyQXacSuRKsbmZg96975538 = KYTVhhyQXacSuRKsbmZg90729349;     KYTVhhyQXacSuRKsbmZg90729349 = KYTVhhyQXacSuRKsbmZg24440309;     KYTVhhyQXacSuRKsbmZg24440309 = KYTVhhyQXacSuRKsbmZg89165801;     KYTVhhyQXacSuRKsbmZg89165801 = KYTVhhyQXacSuRKsbmZg5492512;     KYTVhhyQXacSuRKsbmZg5492512 = KYTVhhyQXacSuRKsbmZg39072789;     KYTVhhyQXacSuRKsbmZg39072789 = KYTVhhyQXacSuRKsbmZg61436182;     KYTVhhyQXacSuRKsbmZg61436182 = KYTVhhyQXacSuRKsbmZg70075097;     KYTVhhyQXacSuRKsbmZg70075097 = KYTVhhyQXacSuRKsbmZg32632410;     KYTVhhyQXacSuRKsbmZg32632410 = KYTVhhyQXacSuRKsbmZg25498384;     KYTVhhyQXacSuRKsbmZg25498384 = KYTVhhyQXacSuRKsbmZg26406930;     KYTVhhyQXacSuRKsbmZg26406930 = KYTVhhyQXacSuRKsbmZg54985568;     KYTVhhyQXacSuRKsbmZg54985568 = KYTVhhyQXacSuRKsbmZg7736179;     KYTVhhyQXacSuRKsbmZg7736179 = KYTVhhyQXacSuRKsbmZg13543196;     KYTVhhyQXacSuRKsbmZg13543196 = KYTVhhyQXacSuRKsbmZg16126905;     KYTVhhyQXacSuRKsbmZg16126905 = KYTVhhyQXacSuRKsbmZg16954992;     KYTVhhyQXacSuRKsbmZg16954992 = KYTVhhyQXacSuRKsbmZg92004284;     KYTVhhyQXacSuRKsbmZg92004284 = KYTVhhyQXacSuRKsbmZg86454096;     KYTVhhyQXacSuRKsbmZg86454096 = KYTVhhyQXacSuRKsbmZg69642623;     KYTVhhyQXacSuRKsbmZg69642623 = KYTVhhyQXacSuRKsbmZg7700490;     KYTVhhyQXacSuRKsbmZg7700490 = KYTVhhyQXacSuRKsbmZg38575080;     KYTVhhyQXacSuRKsbmZg38575080 = KYTVhhyQXacSuRKsbmZg10308882;     KYTVhhyQXacSuRKsbmZg10308882 = KYTVhhyQXacSuRKsbmZg8237313;     KYTVhhyQXacSuRKsbmZg8237313 = KYTVhhyQXacSuRKsbmZg35089151;     KYTVhhyQXacSuRKsbmZg35089151 = KYTVhhyQXacSuRKsbmZg39730562;     KYTVhhyQXacSuRKsbmZg39730562 = KYTVhhyQXacSuRKsbmZg64253861;     KYTVhhyQXacSuRKsbmZg64253861 = KYTVhhyQXacSuRKsbmZg21426465;     KYTVhhyQXacSuRKsbmZg21426465 = KYTVhhyQXacSuRKsbmZg57273889;     KYTVhhyQXacSuRKsbmZg57273889 = KYTVhhyQXacSuRKsbmZg41056721;     KYTVhhyQXacSuRKsbmZg41056721 = KYTVhhyQXacSuRKsbmZg12503572;     KYTVhhyQXacSuRKsbmZg12503572 = KYTVhhyQXacSuRKsbmZg26703937;     KYTVhhyQXacSuRKsbmZg26703937 = KYTVhhyQXacSuRKsbmZg88518717;     KYTVhhyQXacSuRKsbmZg88518717 = KYTVhhyQXacSuRKsbmZg5273152;     KYTVhhyQXacSuRKsbmZg5273152 = KYTVhhyQXacSuRKsbmZg94413970;     KYTVhhyQXacSuRKsbmZg94413970 = KYTVhhyQXacSuRKsbmZg24274718;     KYTVhhyQXacSuRKsbmZg24274718 = KYTVhhyQXacSuRKsbmZg26347737;     KYTVhhyQXacSuRKsbmZg26347737 = KYTVhhyQXacSuRKsbmZg31337888;     KYTVhhyQXacSuRKsbmZg31337888 = KYTVhhyQXacSuRKsbmZg27505682;     KYTVhhyQXacSuRKsbmZg27505682 = KYTVhhyQXacSuRKsbmZg84429263;     KYTVhhyQXacSuRKsbmZg84429263 = KYTVhhyQXacSuRKsbmZg8319635;     KYTVhhyQXacSuRKsbmZg8319635 = KYTVhhyQXacSuRKsbmZg4971676;     KYTVhhyQXacSuRKsbmZg4971676 = KYTVhhyQXacSuRKsbmZg35066633;     KYTVhhyQXacSuRKsbmZg35066633 = KYTVhhyQXacSuRKsbmZg69353923;     KYTVhhyQXacSuRKsbmZg69353923 = KYTVhhyQXacSuRKsbmZg26219065;     KYTVhhyQXacSuRKsbmZg26219065 = KYTVhhyQXacSuRKsbmZg82804556;     KYTVhhyQXacSuRKsbmZg82804556 = KYTVhhyQXacSuRKsbmZg90227413;     KYTVhhyQXacSuRKsbmZg90227413 = KYTVhhyQXacSuRKsbmZg40268456;     KYTVhhyQXacSuRKsbmZg40268456 = KYTVhhyQXacSuRKsbmZg22268285;     KYTVhhyQXacSuRKsbmZg22268285 = KYTVhhyQXacSuRKsbmZg51359332;     KYTVhhyQXacSuRKsbmZg51359332 = KYTVhhyQXacSuRKsbmZg50828987;     KYTVhhyQXacSuRKsbmZg50828987 = KYTVhhyQXacSuRKsbmZg54798599;     KYTVhhyQXacSuRKsbmZg54798599 = KYTVhhyQXacSuRKsbmZg26166806;     KYTVhhyQXacSuRKsbmZg26166806 = KYTVhhyQXacSuRKsbmZg23189677;     KYTVhhyQXacSuRKsbmZg23189677 = KYTVhhyQXacSuRKsbmZg3922867;     KYTVhhyQXacSuRKsbmZg3922867 = KYTVhhyQXacSuRKsbmZg32289019;     KYTVhhyQXacSuRKsbmZg32289019 = KYTVhhyQXacSuRKsbmZg86666656;     KYTVhhyQXacSuRKsbmZg86666656 = KYTVhhyQXacSuRKsbmZg71882075;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KOXNEQleYQYllRVFOdmL35352204() {     double tKZhJosfheoYWgzqAxye97467352 = -617620699;    double tKZhJosfheoYWgzqAxye69411391 = -272615040;    double tKZhJosfheoYWgzqAxye67672627 = -258673901;    double tKZhJosfheoYWgzqAxye78925919 = -553697742;    double tKZhJosfheoYWgzqAxye42075192 = -852812716;    double tKZhJosfheoYWgzqAxye25685045 = -9195779;    double tKZhJosfheoYWgzqAxye81696715 = -163069074;    double tKZhJosfheoYWgzqAxye32045171 = -59264664;    double tKZhJosfheoYWgzqAxye99437639 = -409485129;    double tKZhJosfheoYWgzqAxye62875192 = -917791188;    double tKZhJosfheoYWgzqAxye26628768 = -763591305;    double tKZhJosfheoYWgzqAxye95272417 = -197164626;    double tKZhJosfheoYWgzqAxye20881415 = -882295101;    double tKZhJosfheoYWgzqAxye76503259 = 13046552;    double tKZhJosfheoYWgzqAxye68527383 = 53242183;    double tKZhJosfheoYWgzqAxye64958987 = -726706853;    double tKZhJosfheoYWgzqAxye24558241 = -666046627;    double tKZhJosfheoYWgzqAxye29956604 = -841724483;    double tKZhJosfheoYWgzqAxye20771177 = -652956902;    double tKZhJosfheoYWgzqAxye486305 = -935846457;    double tKZhJosfheoYWgzqAxye82360282 = 88804764;    double tKZhJosfheoYWgzqAxye18399615 = -517532208;    double tKZhJosfheoYWgzqAxye69041867 = -468961574;    double tKZhJosfheoYWgzqAxye22964298 = -314314345;    double tKZhJosfheoYWgzqAxye71980225 = -303656677;    double tKZhJosfheoYWgzqAxye85267332 = -573710559;    double tKZhJosfheoYWgzqAxye99059392 = -500753012;    double tKZhJosfheoYWgzqAxye53820575 = -237157158;    double tKZhJosfheoYWgzqAxye55269636 = -601539156;    double tKZhJosfheoYWgzqAxye19730349 = -903001119;    double tKZhJosfheoYWgzqAxye27188364 = -60027584;    double tKZhJosfheoYWgzqAxye39176265 = -158942707;    double tKZhJosfheoYWgzqAxye72904500 = -764446664;    double tKZhJosfheoYWgzqAxye62798557 = -642456306;    double tKZhJosfheoYWgzqAxye46617969 = -119023213;    double tKZhJosfheoYWgzqAxye52796484 = 44259309;    double tKZhJosfheoYWgzqAxye85456865 = -621737077;    double tKZhJosfheoYWgzqAxye43058923 = -373618085;    double tKZhJosfheoYWgzqAxye8707557 = -460931917;    double tKZhJosfheoYWgzqAxye55586557 = -361067112;    double tKZhJosfheoYWgzqAxye59772183 = -408529362;    double tKZhJosfheoYWgzqAxye36841890 = -967562570;    double tKZhJosfheoYWgzqAxye60823576 = -726935581;    double tKZhJosfheoYWgzqAxye41657378 = -974335813;    double tKZhJosfheoYWgzqAxye88956414 = -493356157;    double tKZhJosfheoYWgzqAxye57129671 = -126138809;    double tKZhJosfheoYWgzqAxye24208763 = -669470999;    double tKZhJosfheoYWgzqAxye97278572 = -515801042;    double tKZhJosfheoYWgzqAxye56303832 = -382345512;    double tKZhJosfheoYWgzqAxye92882784 = -954723186;    double tKZhJosfheoYWgzqAxye71572429 = -983096911;    double tKZhJosfheoYWgzqAxye97333676 = -497545615;    double tKZhJosfheoYWgzqAxye50706013 = -454763970;    double tKZhJosfheoYWgzqAxye94397292 = -998477495;    double tKZhJosfheoYWgzqAxye52991921 = -972261739;    double tKZhJosfheoYWgzqAxye79067738 = -88491;    double tKZhJosfheoYWgzqAxye369524 = -803653467;    double tKZhJosfheoYWgzqAxye44708329 = -944359557;    double tKZhJosfheoYWgzqAxye6945695 = -150041066;    double tKZhJosfheoYWgzqAxye56807859 = -179102158;    double tKZhJosfheoYWgzqAxye26625652 = -508442768;    double tKZhJosfheoYWgzqAxye27876141 = -925911917;    double tKZhJosfheoYWgzqAxye76775535 = -457725508;    double tKZhJosfheoYWgzqAxye79707291 = -506484010;    double tKZhJosfheoYWgzqAxye35686828 = -757763605;    double tKZhJosfheoYWgzqAxye87452502 = -504648598;    double tKZhJosfheoYWgzqAxye22367917 = -432717963;    double tKZhJosfheoYWgzqAxye58082857 = -139838795;    double tKZhJosfheoYWgzqAxye29885291 = -867930235;    double tKZhJosfheoYWgzqAxye15730899 = -991017127;    double tKZhJosfheoYWgzqAxye79502122 = -4969776;    double tKZhJosfheoYWgzqAxye81499318 = -192428543;    double tKZhJosfheoYWgzqAxye21249047 = -280792566;    double tKZhJosfheoYWgzqAxye65184619 = -191889790;    double tKZhJosfheoYWgzqAxye40714122 = -427317095;    double tKZhJosfheoYWgzqAxye45518393 = 56367333;    double tKZhJosfheoYWgzqAxye57576038 = -790596628;    double tKZhJosfheoYWgzqAxye27384490 = -494625761;    double tKZhJosfheoYWgzqAxye34007884 = -820958188;    double tKZhJosfheoYWgzqAxye14850554 = -77517868;    double tKZhJosfheoYWgzqAxye61058570 = -904239560;    double tKZhJosfheoYWgzqAxye1780821 = -984951970;    double tKZhJosfheoYWgzqAxye97516743 = -854811646;    double tKZhJosfheoYWgzqAxye62386851 = -646815970;    double tKZhJosfheoYWgzqAxye48157919 = -919904209;    double tKZhJosfheoYWgzqAxye29854688 = -562481970;    double tKZhJosfheoYWgzqAxye88470251 = -704178738;    double tKZhJosfheoYWgzqAxye78507208 = -765969170;    double tKZhJosfheoYWgzqAxye9806637 = -670194568;    double tKZhJosfheoYWgzqAxye67550231 = -18934723;    double tKZhJosfheoYWgzqAxye52426961 = -152087225;    double tKZhJosfheoYWgzqAxye40748537 = -677377521;    double tKZhJosfheoYWgzqAxye36113228 = -123577019;    double tKZhJosfheoYWgzqAxye51899697 = -181829760;    double tKZhJosfheoYWgzqAxye28960906 = -852624345;    double tKZhJosfheoYWgzqAxye31896042 = -482617446;    double tKZhJosfheoYWgzqAxye60066355 = -409837062;    double tKZhJosfheoYWgzqAxye81116284 = -120451572;    double tKZhJosfheoYWgzqAxye5970550 = -116572209;    double tKZhJosfheoYWgzqAxye1503912 = -617620699;     tKZhJosfheoYWgzqAxye97467352 = tKZhJosfheoYWgzqAxye69411391;     tKZhJosfheoYWgzqAxye69411391 = tKZhJosfheoYWgzqAxye67672627;     tKZhJosfheoYWgzqAxye67672627 = tKZhJosfheoYWgzqAxye78925919;     tKZhJosfheoYWgzqAxye78925919 = tKZhJosfheoYWgzqAxye42075192;     tKZhJosfheoYWgzqAxye42075192 = tKZhJosfheoYWgzqAxye25685045;     tKZhJosfheoYWgzqAxye25685045 = tKZhJosfheoYWgzqAxye81696715;     tKZhJosfheoYWgzqAxye81696715 = tKZhJosfheoYWgzqAxye32045171;     tKZhJosfheoYWgzqAxye32045171 = tKZhJosfheoYWgzqAxye99437639;     tKZhJosfheoYWgzqAxye99437639 = tKZhJosfheoYWgzqAxye62875192;     tKZhJosfheoYWgzqAxye62875192 = tKZhJosfheoYWgzqAxye26628768;     tKZhJosfheoYWgzqAxye26628768 = tKZhJosfheoYWgzqAxye95272417;     tKZhJosfheoYWgzqAxye95272417 = tKZhJosfheoYWgzqAxye20881415;     tKZhJosfheoYWgzqAxye20881415 = tKZhJosfheoYWgzqAxye76503259;     tKZhJosfheoYWgzqAxye76503259 = tKZhJosfheoYWgzqAxye68527383;     tKZhJosfheoYWgzqAxye68527383 = tKZhJosfheoYWgzqAxye64958987;     tKZhJosfheoYWgzqAxye64958987 = tKZhJosfheoYWgzqAxye24558241;     tKZhJosfheoYWgzqAxye24558241 = tKZhJosfheoYWgzqAxye29956604;     tKZhJosfheoYWgzqAxye29956604 = tKZhJosfheoYWgzqAxye20771177;     tKZhJosfheoYWgzqAxye20771177 = tKZhJosfheoYWgzqAxye486305;     tKZhJosfheoYWgzqAxye486305 = tKZhJosfheoYWgzqAxye82360282;     tKZhJosfheoYWgzqAxye82360282 = tKZhJosfheoYWgzqAxye18399615;     tKZhJosfheoYWgzqAxye18399615 = tKZhJosfheoYWgzqAxye69041867;     tKZhJosfheoYWgzqAxye69041867 = tKZhJosfheoYWgzqAxye22964298;     tKZhJosfheoYWgzqAxye22964298 = tKZhJosfheoYWgzqAxye71980225;     tKZhJosfheoYWgzqAxye71980225 = tKZhJosfheoYWgzqAxye85267332;     tKZhJosfheoYWgzqAxye85267332 = tKZhJosfheoYWgzqAxye99059392;     tKZhJosfheoYWgzqAxye99059392 = tKZhJosfheoYWgzqAxye53820575;     tKZhJosfheoYWgzqAxye53820575 = tKZhJosfheoYWgzqAxye55269636;     tKZhJosfheoYWgzqAxye55269636 = tKZhJosfheoYWgzqAxye19730349;     tKZhJosfheoYWgzqAxye19730349 = tKZhJosfheoYWgzqAxye27188364;     tKZhJosfheoYWgzqAxye27188364 = tKZhJosfheoYWgzqAxye39176265;     tKZhJosfheoYWgzqAxye39176265 = tKZhJosfheoYWgzqAxye72904500;     tKZhJosfheoYWgzqAxye72904500 = tKZhJosfheoYWgzqAxye62798557;     tKZhJosfheoYWgzqAxye62798557 = tKZhJosfheoYWgzqAxye46617969;     tKZhJosfheoYWgzqAxye46617969 = tKZhJosfheoYWgzqAxye52796484;     tKZhJosfheoYWgzqAxye52796484 = tKZhJosfheoYWgzqAxye85456865;     tKZhJosfheoYWgzqAxye85456865 = tKZhJosfheoYWgzqAxye43058923;     tKZhJosfheoYWgzqAxye43058923 = tKZhJosfheoYWgzqAxye8707557;     tKZhJosfheoYWgzqAxye8707557 = tKZhJosfheoYWgzqAxye55586557;     tKZhJosfheoYWgzqAxye55586557 = tKZhJosfheoYWgzqAxye59772183;     tKZhJosfheoYWgzqAxye59772183 = tKZhJosfheoYWgzqAxye36841890;     tKZhJosfheoYWgzqAxye36841890 = tKZhJosfheoYWgzqAxye60823576;     tKZhJosfheoYWgzqAxye60823576 = tKZhJosfheoYWgzqAxye41657378;     tKZhJosfheoYWgzqAxye41657378 = tKZhJosfheoYWgzqAxye88956414;     tKZhJosfheoYWgzqAxye88956414 = tKZhJosfheoYWgzqAxye57129671;     tKZhJosfheoYWgzqAxye57129671 = tKZhJosfheoYWgzqAxye24208763;     tKZhJosfheoYWgzqAxye24208763 = tKZhJosfheoYWgzqAxye97278572;     tKZhJosfheoYWgzqAxye97278572 = tKZhJosfheoYWgzqAxye56303832;     tKZhJosfheoYWgzqAxye56303832 = tKZhJosfheoYWgzqAxye92882784;     tKZhJosfheoYWgzqAxye92882784 = tKZhJosfheoYWgzqAxye71572429;     tKZhJosfheoYWgzqAxye71572429 = tKZhJosfheoYWgzqAxye97333676;     tKZhJosfheoYWgzqAxye97333676 = tKZhJosfheoYWgzqAxye50706013;     tKZhJosfheoYWgzqAxye50706013 = tKZhJosfheoYWgzqAxye94397292;     tKZhJosfheoYWgzqAxye94397292 = tKZhJosfheoYWgzqAxye52991921;     tKZhJosfheoYWgzqAxye52991921 = tKZhJosfheoYWgzqAxye79067738;     tKZhJosfheoYWgzqAxye79067738 = tKZhJosfheoYWgzqAxye369524;     tKZhJosfheoYWgzqAxye369524 = tKZhJosfheoYWgzqAxye44708329;     tKZhJosfheoYWgzqAxye44708329 = tKZhJosfheoYWgzqAxye6945695;     tKZhJosfheoYWgzqAxye6945695 = tKZhJosfheoYWgzqAxye56807859;     tKZhJosfheoYWgzqAxye56807859 = tKZhJosfheoYWgzqAxye26625652;     tKZhJosfheoYWgzqAxye26625652 = tKZhJosfheoYWgzqAxye27876141;     tKZhJosfheoYWgzqAxye27876141 = tKZhJosfheoYWgzqAxye76775535;     tKZhJosfheoYWgzqAxye76775535 = tKZhJosfheoYWgzqAxye79707291;     tKZhJosfheoYWgzqAxye79707291 = tKZhJosfheoYWgzqAxye35686828;     tKZhJosfheoYWgzqAxye35686828 = tKZhJosfheoYWgzqAxye87452502;     tKZhJosfheoYWgzqAxye87452502 = tKZhJosfheoYWgzqAxye22367917;     tKZhJosfheoYWgzqAxye22367917 = tKZhJosfheoYWgzqAxye58082857;     tKZhJosfheoYWgzqAxye58082857 = tKZhJosfheoYWgzqAxye29885291;     tKZhJosfheoYWgzqAxye29885291 = tKZhJosfheoYWgzqAxye15730899;     tKZhJosfheoYWgzqAxye15730899 = tKZhJosfheoYWgzqAxye79502122;     tKZhJosfheoYWgzqAxye79502122 = tKZhJosfheoYWgzqAxye81499318;     tKZhJosfheoYWgzqAxye81499318 = tKZhJosfheoYWgzqAxye21249047;     tKZhJosfheoYWgzqAxye21249047 = tKZhJosfheoYWgzqAxye65184619;     tKZhJosfheoYWgzqAxye65184619 = tKZhJosfheoYWgzqAxye40714122;     tKZhJosfheoYWgzqAxye40714122 = tKZhJosfheoYWgzqAxye45518393;     tKZhJosfheoYWgzqAxye45518393 = tKZhJosfheoYWgzqAxye57576038;     tKZhJosfheoYWgzqAxye57576038 = tKZhJosfheoYWgzqAxye27384490;     tKZhJosfheoYWgzqAxye27384490 = tKZhJosfheoYWgzqAxye34007884;     tKZhJosfheoYWgzqAxye34007884 = tKZhJosfheoYWgzqAxye14850554;     tKZhJosfheoYWgzqAxye14850554 = tKZhJosfheoYWgzqAxye61058570;     tKZhJosfheoYWgzqAxye61058570 = tKZhJosfheoYWgzqAxye1780821;     tKZhJosfheoYWgzqAxye1780821 = tKZhJosfheoYWgzqAxye97516743;     tKZhJosfheoYWgzqAxye97516743 = tKZhJosfheoYWgzqAxye62386851;     tKZhJosfheoYWgzqAxye62386851 = tKZhJosfheoYWgzqAxye48157919;     tKZhJosfheoYWgzqAxye48157919 = tKZhJosfheoYWgzqAxye29854688;     tKZhJosfheoYWgzqAxye29854688 = tKZhJosfheoYWgzqAxye88470251;     tKZhJosfheoYWgzqAxye88470251 = tKZhJosfheoYWgzqAxye78507208;     tKZhJosfheoYWgzqAxye78507208 = tKZhJosfheoYWgzqAxye9806637;     tKZhJosfheoYWgzqAxye9806637 = tKZhJosfheoYWgzqAxye67550231;     tKZhJosfheoYWgzqAxye67550231 = tKZhJosfheoYWgzqAxye52426961;     tKZhJosfheoYWgzqAxye52426961 = tKZhJosfheoYWgzqAxye40748537;     tKZhJosfheoYWgzqAxye40748537 = tKZhJosfheoYWgzqAxye36113228;     tKZhJosfheoYWgzqAxye36113228 = tKZhJosfheoYWgzqAxye51899697;     tKZhJosfheoYWgzqAxye51899697 = tKZhJosfheoYWgzqAxye28960906;     tKZhJosfheoYWgzqAxye28960906 = tKZhJosfheoYWgzqAxye31896042;     tKZhJosfheoYWgzqAxye31896042 = tKZhJosfheoYWgzqAxye60066355;     tKZhJosfheoYWgzqAxye60066355 = tKZhJosfheoYWgzqAxye81116284;     tKZhJosfheoYWgzqAxye81116284 = tKZhJosfheoYWgzqAxye5970550;     tKZhJosfheoYWgzqAxye5970550 = tKZhJosfheoYWgzqAxye1503912;     tKZhJosfheoYWgzqAxye1503912 = tKZhJosfheoYWgzqAxye97467352;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NMzBwHmigazKtySFfzaX62946460() {     double szmKhqsVAUdFAlZlQxrM23052630 = -42653800;    double szmKhqsVAUdFAlZlQxrM65808534 = -528092817;    double szmKhqsVAUdFAlZlQxrM551803 = -61135437;    double szmKhqsVAUdFAlZlQxrM24647849 = -561020636;    double szmKhqsVAUdFAlZlQxrM11417197 = -899735003;    double szmKhqsVAUdFAlZlQxrM42694321 = -164176782;    double szmKhqsVAUdFAlZlQxrM87017560 = -745340281;    double szmKhqsVAUdFAlZlQxrM47055295 = -439946543;    double szmKhqsVAUdFAlZlQxrM24766933 = -905709174;    double szmKhqsVAUdFAlZlQxrM82033574 = -152352712;    double szmKhqsVAUdFAlZlQxrM40962321 = 25980809;    double szmKhqsVAUdFAlZlQxrM30590073 = -640873533;    double szmKhqsVAUdFAlZlQxrM97462192 = -646543466;    double szmKhqsVAUdFAlZlQxrM68062976 = 38522259;    double szmKhqsVAUdFAlZlQxrM24796271 = -935303775;    double szmKhqsVAUdFAlZlQxrM72680030 = -62898330;    double szmKhqsVAUdFAlZlQxrM24356358 = -354366421;    double szmKhqsVAUdFAlZlQxrM51072508 = -258001843;    double szmKhqsVAUdFAlZlQxrM82235897 = -335930759;    double szmKhqsVAUdFAlZlQxrM61647771 = -910285832;    double szmKhqsVAUdFAlZlQxrM83369548 = -372895017;    double szmKhqsVAUdFAlZlQxrM19902722 = -937265138;    double szmKhqsVAUdFAlZlQxrM72805665 = -513053679;    double szmKhqsVAUdFAlZlQxrM24678341 = -173163324;    double szmKhqsVAUdFAlZlQxrM26883364 = -971152819;    double szmKhqsVAUdFAlZlQxrM14756469 = -138161101;    double szmKhqsVAUdFAlZlQxrM81447301 = -66566147;    double szmKhqsVAUdFAlZlQxrM17719375 = -466819391;    double szmKhqsVAUdFAlZlQxrM63146848 = -837833006;    double szmKhqsVAUdFAlZlQxrM73052839 = -811910681;    double szmKhqsVAUdFAlZlQxrM49234999 = -938674615;    double szmKhqsVAUdFAlZlQxrM76366197 = -432738541;    double szmKhqsVAUdFAlZlQxrM94091552 = -112267801;    double szmKhqsVAUdFAlZlQxrM16385629 = -698601278;    double szmKhqsVAUdFAlZlQxrM48022957 = -820459125;    double szmKhqsVAUdFAlZlQxrM57588335 = -473335407;    double szmKhqsVAUdFAlZlQxrM35102251 = -2249022;    double szmKhqsVAUdFAlZlQxrM18631610 = -222332910;    double szmKhqsVAUdFAlZlQxrM49631135 = -919270239;    double szmKhqsVAUdFAlZlQxrM64370231 = -831037431;    double szmKhqsVAUdFAlZlQxrM6923464 = -983589552;    double szmKhqsVAUdFAlZlQxrM80851480 = -376601074;    double szmKhqsVAUdFAlZlQxrM10023796 = -755541963;    double szmKhqsVAUdFAlZlQxrM12450658 = -342751412;    double szmKhqsVAUdFAlZlQxrM80937290 = -432377274;    double szmKhqsVAUdFAlZlQxrM23529994 = -648686346;    double szmKhqsVAUdFAlZlQxrM23977217 = -701939860;    double szmKhqsVAUdFAlZlQxrM5391344 = -708983889;    double szmKhqsVAUdFAlZlQxrM7115152 = -482021739;    double szmKhqsVAUdFAlZlQxrM46692779 = -802746339;    double szmKhqsVAUdFAlZlQxrM81708676 = -131755513;    double szmKhqsVAUdFAlZlQxrM24592256 = -675624794;    double szmKhqsVAUdFAlZlQxrM68779617 = 77337481;    double szmKhqsVAUdFAlZlQxrM63296201 = -213541276;    double szmKhqsVAUdFAlZlQxrM79576912 = -347352156;    double szmKhqsVAUdFAlZlQxrM3149909 = -105388663;    double szmKhqsVAUdFAlZlQxrM93002869 = 84960862;    double szmKhqsVAUdFAlZlQxrM75873462 = -887972114;    double szmKhqsVAUdFAlZlQxrM97764485 = -589867817;    double szmKhqsVAUdFAlZlQxrM96660727 = -661573903;    double szmKhqsVAUdFAlZlQxrM61247019 = 2389364;    double szmKhqsVAUdFAlZlQxrM69298185 = -178520891;    double szmKhqsVAUdFAlZlQxrM83908447 = -602113537;    double szmKhqsVAUdFAlZlQxrM51714093 = 6201507;    double szmKhqsVAUdFAlZlQxrM32798576 = -213678097;    double szmKhqsVAUdFAlZlQxrM64596124 = -541280651;    double szmKhqsVAUdFAlZlQxrM36498521 = -428605732;    double szmKhqsVAUdFAlZlQxrM81076563 = -947942189;    double szmKhqsVAUdFAlZlQxrM20040020 = -141018617;    double szmKhqsVAUdFAlZlQxrM67207936 = -361968369;    double szmKhqsVAUdFAlZlQxrM37577780 = 39350692;    double szmKhqsVAUdFAlZlQxrM5724748 = -32033512;    double szmKhqsVAUdFAlZlQxrM1441373 = -338731605;    double szmKhqsVAUdFAlZlQxrM17865666 = -504893329;    double szmKhqsVAUdFAlZlQxrM54724308 = -926696281;    double szmKhqsVAUdFAlZlQxrM2518069 = -996293944;    double szmKhqsVAUdFAlZlQxrM9878926 = -81723176;    double szmKhqsVAUdFAlZlQxrM60355008 = -70302268;    double szmKhqsVAUdFAlZlQxrM43741050 = -740786051;    double szmKhqsVAUdFAlZlQxrM3353371 = -222466473;    double szmKhqsVAUdFAlZlQxrM90779252 = -436221242;    double szmKhqsVAUdFAlZlQxrM76055958 = -357582258;    double szmKhqsVAUdFAlZlQxrM10604224 = -984797652;    double szmKhqsVAUdFAlZlQxrM16454069 = 64913332;    double szmKhqsVAUdFAlZlQxrM91344162 = -580155168;    double szmKhqsVAUdFAlZlQxrM24642743 = -163049821;    double szmKhqsVAUdFAlZlQxrM7586581 = -410076023;    double szmKhqsVAUdFAlZlQxrM30795352 = -898726526;    double szmKhqsVAUdFAlZlQxrM36808716 = -251249122;    double szmKhqsVAUdFAlZlQxrM44873049 = -615070463;    double szmKhqsVAUdFAlZlQxrM64585465 = -458296269;    double szmKhqsVAUdFAlZlQxrM59228789 = -114276909;    double szmKhqsVAUdFAlZlQxrM20867125 = -632465093;    double szmKhqsVAUdFAlZlQxrM52970408 = -157696337;    double szmKhqsVAUdFAlZlQxrM3123212 = -733426795;    double szmKhqsVAUdFAlZlQxrM37625279 = -705068661;    double szmKhqsVAUdFAlZlQxrM96943033 = -774487537;    double szmKhqsVAUdFAlZlQxrM58309702 = -661743470;    double szmKhqsVAUdFAlZlQxrM79652081 = -29073315;    double szmKhqsVAUdFAlZlQxrM16341166 = -42653800;     szmKhqsVAUdFAlZlQxrM23052630 = szmKhqsVAUdFAlZlQxrM65808534;     szmKhqsVAUdFAlZlQxrM65808534 = szmKhqsVAUdFAlZlQxrM551803;     szmKhqsVAUdFAlZlQxrM551803 = szmKhqsVAUdFAlZlQxrM24647849;     szmKhqsVAUdFAlZlQxrM24647849 = szmKhqsVAUdFAlZlQxrM11417197;     szmKhqsVAUdFAlZlQxrM11417197 = szmKhqsVAUdFAlZlQxrM42694321;     szmKhqsVAUdFAlZlQxrM42694321 = szmKhqsVAUdFAlZlQxrM87017560;     szmKhqsVAUdFAlZlQxrM87017560 = szmKhqsVAUdFAlZlQxrM47055295;     szmKhqsVAUdFAlZlQxrM47055295 = szmKhqsVAUdFAlZlQxrM24766933;     szmKhqsVAUdFAlZlQxrM24766933 = szmKhqsVAUdFAlZlQxrM82033574;     szmKhqsVAUdFAlZlQxrM82033574 = szmKhqsVAUdFAlZlQxrM40962321;     szmKhqsVAUdFAlZlQxrM40962321 = szmKhqsVAUdFAlZlQxrM30590073;     szmKhqsVAUdFAlZlQxrM30590073 = szmKhqsVAUdFAlZlQxrM97462192;     szmKhqsVAUdFAlZlQxrM97462192 = szmKhqsVAUdFAlZlQxrM68062976;     szmKhqsVAUdFAlZlQxrM68062976 = szmKhqsVAUdFAlZlQxrM24796271;     szmKhqsVAUdFAlZlQxrM24796271 = szmKhqsVAUdFAlZlQxrM72680030;     szmKhqsVAUdFAlZlQxrM72680030 = szmKhqsVAUdFAlZlQxrM24356358;     szmKhqsVAUdFAlZlQxrM24356358 = szmKhqsVAUdFAlZlQxrM51072508;     szmKhqsVAUdFAlZlQxrM51072508 = szmKhqsVAUdFAlZlQxrM82235897;     szmKhqsVAUdFAlZlQxrM82235897 = szmKhqsVAUdFAlZlQxrM61647771;     szmKhqsVAUdFAlZlQxrM61647771 = szmKhqsVAUdFAlZlQxrM83369548;     szmKhqsVAUdFAlZlQxrM83369548 = szmKhqsVAUdFAlZlQxrM19902722;     szmKhqsVAUdFAlZlQxrM19902722 = szmKhqsVAUdFAlZlQxrM72805665;     szmKhqsVAUdFAlZlQxrM72805665 = szmKhqsVAUdFAlZlQxrM24678341;     szmKhqsVAUdFAlZlQxrM24678341 = szmKhqsVAUdFAlZlQxrM26883364;     szmKhqsVAUdFAlZlQxrM26883364 = szmKhqsVAUdFAlZlQxrM14756469;     szmKhqsVAUdFAlZlQxrM14756469 = szmKhqsVAUdFAlZlQxrM81447301;     szmKhqsVAUdFAlZlQxrM81447301 = szmKhqsVAUdFAlZlQxrM17719375;     szmKhqsVAUdFAlZlQxrM17719375 = szmKhqsVAUdFAlZlQxrM63146848;     szmKhqsVAUdFAlZlQxrM63146848 = szmKhqsVAUdFAlZlQxrM73052839;     szmKhqsVAUdFAlZlQxrM73052839 = szmKhqsVAUdFAlZlQxrM49234999;     szmKhqsVAUdFAlZlQxrM49234999 = szmKhqsVAUdFAlZlQxrM76366197;     szmKhqsVAUdFAlZlQxrM76366197 = szmKhqsVAUdFAlZlQxrM94091552;     szmKhqsVAUdFAlZlQxrM94091552 = szmKhqsVAUdFAlZlQxrM16385629;     szmKhqsVAUdFAlZlQxrM16385629 = szmKhqsVAUdFAlZlQxrM48022957;     szmKhqsVAUdFAlZlQxrM48022957 = szmKhqsVAUdFAlZlQxrM57588335;     szmKhqsVAUdFAlZlQxrM57588335 = szmKhqsVAUdFAlZlQxrM35102251;     szmKhqsVAUdFAlZlQxrM35102251 = szmKhqsVAUdFAlZlQxrM18631610;     szmKhqsVAUdFAlZlQxrM18631610 = szmKhqsVAUdFAlZlQxrM49631135;     szmKhqsVAUdFAlZlQxrM49631135 = szmKhqsVAUdFAlZlQxrM64370231;     szmKhqsVAUdFAlZlQxrM64370231 = szmKhqsVAUdFAlZlQxrM6923464;     szmKhqsVAUdFAlZlQxrM6923464 = szmKhqsVAUdFAlZlQxrM80851480;     szmKhqsVAUdFAlZlQxrM80851480 = szmKhqsVAUdFAlZlQxrM10023796;     szmKhqsVAUdFAlZlQxrM10023796 = szmKhqsVAUdFAlZlQxrM12450658;     szmKhqsVAUdFAlZlQxrM12450658 = szmKhqsVAUdFAlZlQxrM80937290;     szmKhqsVAUdFAlZlQxrM80937290 = szmKhqsVAUdFAlZlQxrM23529994;     szmKhqsVAUdFAlZlQxrM23529994 = szmKhqsVAUdFAlZlQxrM23977217;     szmKhqsVAUdFAlZlQxrM23977217 = szmKhqsVAUdFAlZlQxrM5391344;     szmKhqsVAUdFAlZlQxrM5391344 = szmKhqsVAUdFAlZlQxrM7115152;     szmKhqsVAUdFAlZlQxrM7115152 = szmKhqsVAUdFAlZlQxrM46692779;     szmKhqsVAUdFAlZlQxrM46692779 = szmKhqsVAUdFAlZlQxrM81708676;     szmKhqsVAUdFAlZlQxrM81708676 = szmKhqsVAUdFAlZlQxrM24592256;     szmKhqsVAUdFAlZlQxrM24592256 = szmKhqsVAUdFAlZlQxrM68779617;     szmKhqsVAUdFAlZlQxrM68779617 = szmKhqsVAUdFAlZlQxrM63296201;     szmKhqsVAUdFAlZlQxrM63296201 = szmKhqsVAUdFAlZlQxrM79576912;     szmKhqsVAUdFAlZlQxrM79576912 = szmKhqsVAUdFAlZlQxrM3149909;     szmKhqsVAUdFAlZlQxrM3149909 = szmKhqsVAUdFAlZlQxrM93002869;     szmKhqsVAUdFAlZlQxrM93002869 = szmKhqsVAUdFAlZlQxrM75873462;     szmKhqsVAUdFAlZlQxrM75873462 = szmKhqsVAUdFAlZlQxrM97764485;     szmKhqsVAUdFAlZlQxrM97764485 = szmKhqsVAUdFAlZlQxrM96660727;     szmKhqsVAUdFAlZlQxrM96660727 = szmKhqsVAUdFAlZlQxrM61247019;     szmKhqsVAUdFAlZlQxrM61247019 = szmKhqsVAUdFAlZlQxrM69298185;     szmKhqsVAUdFAlZlQxrM69298185 = szmKhqsVAUdFAlZlQxrM83908447;     szmKhqsVAUdFAlZlQxrM83908447 = szmKhqsVAUdFAlZlQxrM51714093;     szmKhqsVAUdFAlZlQxrM51714093 = szmKhqsVAUdFAlZlQxrM32798576;     szmKhqsVAUdFAlZlQxrM32798576 = szmKhqsVAUdFAlZlQxrM64596124;     szmKhqsVAUdFAlZlQxrM64596124 = szmKhqsVAUdFAlZlQxrM36498521;     szmKhqsVAUdFAlZlQxrM36498521 = szmKhqsVAUdFAlZlQxrM81076563;     szmKhqsVAUdFAlZlQxrM81076563 = szmKhqsVAUdFAlZlQxrM20040020;     szmKhqsVAUdFAlZlQxrM20040020 = szmKhqsVAUdFAlZlQxrM67207936;     szmKhqsVAUdFAlZlQxrM67207936 = szmKhqsVAUdFAlZlQxrM37577780;     szmKhqsVAUdFAlZlQxrM37577780 = szmKhqsVAUdFAlZlQxrM5724748;     szmKhqsVAUdFAlZlQxrM5724748 = szmKhqsVAUdFAlZlQxrM1441373;     szmKhqsVAUdFAlZlQxrM1441373 = szmKhqsVAUdFAlZlQxrM17865666;     szmKhqsVAUdFAlZlQxrM17865666 = szmKhqsVAUdFAlZlQxrM54724308;     szmKhqsVAUdFAlZlQxrM54724308 = szmKhqsVAUdFAlZlQxrM2518069;     szmKhqsVAUdFAlZlQxrM2518069 = szmKhqsVAUdFAlZlQxrM9878926;     szmKhqsVAUdFAlZlQxrM9878926 = szmKhqsVAUdFAlZlQxrM60355008;     szmKhqsVAUdFAlZlQxrM60355008 = szmKhqsVAUdFAlZlQxrM43741050;     szmKhqsVAUdFAlZlQxrM43741050 = szmKhqsVAUdFAlZlQxrM3353371;     szmKhqsVAUdFAlZlQxrM3353371 = szmKhqsVAUdFAlZlQxrM90779252;     szmKhqsVAUdFAlZlQxrM90779252 = szmKhqsVAUdFAlZlQxrM76055958;     szmKhqsVAUdFAlZlQxrM76055958 = szmKhqsVAUdFAlZlQxrM10604224;     szmKhqsVAUdFAlZlQxrM10604224 = szmKhqsVAUdFAlZlQxrM16454069;     szmKhqsVAUdFAlZlQxrM16454069 = szmKhqsVAUdFAlZlQxrM91344162;     szmKhqsVAUdFAlZlQxrM91344162 = szmKhqsVAUdFAlZlQxrM24642743;     szmKhqsVAUdFAlZlQxrM24642743 = szmKhqsVAUdFAlZlQxrM7586581;     szmKhqsVAUdFAlZlQxrM7586581 = szmKhqsVAUdFAlZlQxrM30795352;     szmKhqsVAUdFAlZlQxrM30795352 = szmKhqsVAUdFAlZlQxrM36808716;     szmKhqsVAUdFAlZlQxrM36808716 = szmKhqsVAUdFAlZlQxrM44873049;     szmKhqsVAUdFAlZlQxrM44873049 = szmKhqsVAUdFAlZlQxrM64585465;     szmKhqsVAUdFAlZlQxrM64585465 = szmKhqsVAUdFAlZlQxrM59228789;     szmKhqsVAUdFAlZlQxrM59228789 = szmKhqsVAUdFAlZlQxrM20867125;     szmKhqsVAUdFAlZlQxrM20867125 = szmKhqsVAUdFAlZlQxrM52970408;     szmKhqsVAUdFAlZlQxrM52970408 = szmKhqsVAUdFAlZlQxrM3123212;     szmKhqsVAUdFAlZlQxrM3123212 = szmKhqsVAUdFAlZlQxrM37625279;     szmKhqsVAUdFAlZlQxrM37625279 = szmKhqsVAUdFAlZlQxrM96943033;     szmKhqsVAUdFAlZlQxrM96943033 = szmKhqsVAUdFAlZlQxrM58309702;     szmKhqsVAUdFAlZlQxrM58309702 = szmKhqsVAUdFAlZlQxrM79652081;     szmKhqsVAUdFAlZlQxrM79652081 = szmKhqsVAUdFAlZlQxrM16341166;     szmKhqsVAUdFAlZlQxrM16341166 = szmKhqsVAUdFAlZlQxrM23052630;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vKDRBPwgEaMHVQiVdgWq90540717() {     double OUKZMFFKnUEooqxWVZMI48637907 = -567686900;    double OUKZMFFKnUEooqxWVZMI62205678 = -783570594;    double OUKZMFFKnUEooqxWVZMI33430978 = -963596973;    double OUKZMFFKnUEooqxWVZMI70369779 = -568343529;    double OUKZMFFKnUEooqxWVZMI80759201 = -946657290;    double OUKZMFFKnUEooqxWVZMI59703597 = -319157786;    double OUKZMFFKnUEooqxWVZMI92338404 = -227611488;    double OUKZMFFKnUEooqxWVZMI62065419 = -820628422;    double OUKZMFFKnUEooqxWVZMI50096225 = -301933218;    double OUKZMFFKnUEooqxWVZMI1191958 = -486914235;    double OUKZMFFKnUEooqxWVZMI55295875 = -284447078;    double OUKZMFFKnUEooqxWVZMI65907729 = 15417560;    double OUKZMFFKnUEooqxWVZMI74042970 = -410791831;    double OUKZMFFKnUEooqxWVZMI59622693 = 63997965;    double OUKZMFFKnUEooqxWVZMI81065159 = -823849733;    double OUKZMFFKnUEooqxWVZMI80401073 = -499089807;    double OUKZMFFKnUEooqxWVZMI24154474 = -42686215;    double OUKZMFFKnUEooqxWVZMI72188412 = -774279204;    double OUKZMFFKnUEooqxWVZMI43700618 = -18904617;    double OUKZMFFKnUEooqxWVZMI22809239 = -884725208;    double OUKZMFFKnUEooqxWVZMI84378813 = -834594798;    double OUKZMFFKnUEooqxWVZMI21405828 = -256998067;    double OUKZMFFKnUEooqxWVZMI76569463 = -557145784;    double OUKZMFFKnUEooqxWVZMI26392383 = -32012304;    double OUKZMFFKnUEooqxWVZMI81786503 = -538648961;    double OUKZMFFKnUEooqxWVZMI44245606 = -802611642;    double OUKZMFFKnUEooqxWVZMI63835210 = -732379281;    double OUKZMFFKnUEooqxWVZMI81618175 = -696481623;    double OUKZMFFKnUEooqxWVZMI71024060 = 25873143;    double OUKZMFFKnUEooqxWVZMI26375330 = -720820243;    double OUKZMFFKnUEooqxWVZMI71281633 = -717321646;    double OUKZMFFKnUEooqxWVZMI13556130 = -706534375;    double OUKZMFFKnUEooqxWVZMI15278605 = -560088938;    double OUKZMFFKnUEooqxWVZMI69972700 = -754746249;    double OUKZMFFKnUEooqxWVZMI49427945 = -421895036;    double OUKZMFFKnUEooqxWVZMI62380185 = -990930123;    double OUKZMFFKnUEooqxWVZMI84747636 = -482760967;    double OUKZMFFKnUEooqxWVZMI94204296 = -71047735;    double OUKZMFFKnUEooqxWVZMI90554714 = -277608561;    double OUKZMFFKnUEooqxWVZMI73153905 = -201007750;    double OUKZMFFKnUEooqxWVZMI54074745 = -458649742;    double OUKZMFFKnUEooqxWVZMI24861070 = -885639578;    double OUKZMFFKnUEooqxWVZMI59224015 = -784148344;    double OUKZMFFKnUEooqxWVZMI83243936 = -811167011;    double OUKZMFFKnUEooqxWVZMI72918166 = -371398391;    double OUKZMFFKnUEooqxWVZMI89930315 = -71233884;    double OUKZMFFKnUEooqxWVZMI23745671 = -734408720;    double OUKZMFFKnUEooqxWVZMI13504115 = -902166735;    double OUKZMFFKnUEooqxWVZMI57926472 = -581697966;    double OUKZMFFKnUEooqxWVZMI502775 = -650769492;    double OUKZMFFKnUEooqxWVZMI91844924 = -380414116;    double OUKZMFFKnUEooqxWVZMI51850836 = -853703974;    double OUKZMFFKnUEooqxWVZMI86853220 = -490561067;    double OUKZMFFKnUEooqxWVZMI32195110 = -528605058;    double OUKZMFFKnUEooqxWVZMI6161904 = -822442574;    double OUKZMFFKnUEooqxWVZMI27232079 = -210688834;    double OUKZMFFKnUEooqxWVZMI85636214 = -126424810;    double OUKZMFFKnUEooqxWVZMI7038596 = -831584670;    double OUKZMFFKnUEooqxWVZMI88583275 = 70305432;    double OUKZMFFKnUEooqxWVZMI36513596 = -44045649;    double OUKZMFFKnUEooqxWVZMI95868386 = -586778505;    double OUKZMFFKnUEooqxWVZMI10720230 = -531129865;    double OUKZMFFKnUEooqxWVZMI91041358 = -746501566;    double OUKZMFFKnUEooqxWVZMI23720896 = -581112976;    double OUKZMFFKnUEooqxWVZMI29910324 = -769592590;    double OUKZMFFKnUEooqxWVZMI41739746 = -577912704;    double OUKZMFFKnUEooqxWVZMI50629125 = -424493502;    double OUKZMFFKnUEooqxWVZMI4070271 = -656045583;    double OUKZMFFKnUEooqxWVZMI10194748 = -514106999;    double OUKZMFFKnUEooqxWVZMI18684975 = -832919610;    double OUKZMFFKnUEooqxWVZMI95653437 = 83671159;    double OUKZMFFKnUEooqxWVZMI29950177 = -971638481;    double OUKZMFFKnUEooqxWVZMI81633698 = -396670643;    double OUKZMFFKnUEooqxWVZMI70546713 = -817896867;    double OUKZMFFKnUEooqxWVZMI68734493 = -326075467;    double OUKZMFFKnUEooqxWVZMI59517743 = -948955221;    double OUKZMFFKnUEooqxWVZMI62181813 = -472849724;    double OUKZMFFKnUEooqxWVZMI93325527 = -745978774;    double OUKZMFFKnUEooqxWVZMI53474216 = -660613913;    double OUKZMFFKnUEooqxWVZMI91856187 = -367415078;    double OUKZMFFKnUEooqxWVZMI20499935 = 31797077;    double OUKZMFFKnUEooqxWVZMI50331096 = -830212546;    double OUKZMFFKnUEooqxWVZMI23691704 = -14783658;    double OUKZMFFKnUEooqxWVZMI70521286 = -323357365;    double OUKZMFFKnUEooqxWVZMI34530406 = -240406128;    double OUKZMFFKnUEooqxWVZMI19430798 = -863617672;    double OUKZMFFKnUEooqxWVZMI26702909 = -115973308;    double OUKZMFFKnUEooqxWVZMI83083494 = 68516119;    double OUKZMFFKnUEooqxWVZMI63810796 = -932303676;    double OUKZMFFKnUEooqxWVZMI22195867 = -111206203;    double OUKZMFFKnUEooqxWVZMI76743970 = -764505313;    double OUKZMFFKnUEooqxWVZMI77709041 = -651176297;    double OUKZMFFKnUEooqxWVZMI5621021 = -41353167;    double OUKZMFFKnUEooqxWVZMI54041118 = -133562913;    double OUKZMFFKnUEooqxWVZMI77285518 = -614229246;    double OUKZMFFKnUEooqxWVZMI43354516 = -927519877;    double OUKZMFFKnUEooqxWVZMI33819711 = -39138013;    double OUKZMFFKnUEooqxWVZMI35503120 = -103035369;    double OUKZMFFKnUEooqxWVZMI53333612 = 58425579;    double OUKZMFFKnUEooqxWVZMI31178421 = -567686900;     OUKZMFFKnUEooqxWVZMI48637907 = OUKZMFFKnUEooqxWVZMI62205678;     OUKZMFFKnUEooqxWVZMI62205678 = OUKZMFFKnUEooqxWVZMI33430978;     OUKZMFFKnUEooqxWVZMI33430978 = OUKZMFFKnUEooqxWVZMI70369779;     OUKZMFFKnUEooqxWVZMI70369779 = OUKZMFFKnUEooqxWVZMI80759201;     OUKZMFFKnUEooqxWVZMI80759201 = OUKZMFFKnUEooqxWVZMI59703597;     OUKZMFFKnUEooqxWVZMI59703597 = OUKZMFFKnUEooqxWVZMI92338404;     OUKZMFFKnUEooqxWVZMI92338404 = OUKZMFFKnUEooqxWVZMI62065419;     OUKZMFFKnUEooqxWVZMI62065419 = OUKZMFFKnUEooqxWVZMI50096225;     OUKZMFFKnUEooqxWVZMI50096225 = OUKZMFFKnUEooqxWVZMI1191958;     OUKZMFFKnUEooqxWVZMI1191958 = OUKZMFFKnUEooqxWVZMI55295875;     OUKZMFFKnUEooqxWVZMI55295875 = OUKZMFFKnUEooqxWVZMI65907729;     OUKZMFFKnUEooqxWVZMI65907729 = OUKZMFFKnUEooqxWVZMI74042970;     OUKZMFFKnUEooqxWVZMI74042970 = OUKZMFFKnUEooqxWVZMI59622693;     OUKZMFFKnUEooqxWVZMI59622693 = OUKZMFFKnUEooqxWVZMI81065159;     OUKZMFFKnUEooqxWVZMI81065159 = OUKZMFFKnUEooqxWVZMI80401073;     OUKZMFFKnUEooqxWVZMI80401073 = OUKZMFFKnUEooqxWVZMI24154474;     OUKZMFFKnUEooqxWVZMI24154474 = OUKZMFFKnUEooqxWVZMI72188412;     OUKZMFFKnUEooqxWVZMI72188412 = OUKZMFFKnUEooqxWVZMI43700618;     OUKZMFFKnUEooqxWVZMI43700618 = OUKZMFFKnUEooqxWVZMI22809239;     OUKZMFFKnUEooqxWVZMI22809239 = OUKZMFFKnUEooqxWVZMI84378813;     OUKZMFFKnUEooqxWVZMI84378813 = OUKZMFFKnUEooqxWVZMI21405828;     OUKZMFFKnUEooqxWVZMI21405828 = OUKZMFFKnUEooqxWVZMI76569463;     OUKZMFFKnUEooqxWVZMI76569463 = OUKZMFFKnUEooqxWVZMI26392383;     OUKZMFFKnUEooqxWVZMI26392383 = OUKZMFFKnUEooqxWVZMI81786503;     OUKZMFFKnUEooqxWVZMI81786503 = OUKZMFFKnUEooqxWVZMI44245606;     OUKZMFFKnUEooqxWVZMI44245606 = OUKZMFFKnUEooqxWVZMI63835210;     OUKZMFFKnUEooqxWVZMI63835210 = OUKZMFFKnUEooqxWVZMI81618175;     OUKZMFFKnUEooqxWVZMI81618175 = OUKZMFFKnUEooqxWVZMI71024060;     OUKZMFFKnUEooqxWVZMI71024060 = OUKZMFFKnUEooqxWVZMI26375330;     OUKZMFFKnUEooqxWVZMI26375330 = OUKZMFFKnUEooqxWVZMI71281633;     OUKZMFFKnUEooqxWVZMI71281633 = OUKZMFFKnUEooqxWVZMI13556130;     OUKZMFFKnUEooqxWVZMI13556130 = OUKZMFFKnUEooqxWVZMI15278605;     OUKZMFFKnUEooqxWVZMI15278605 = OUKZMFFKnUEooqxWVZMI69972700;     OUKZMFFKnUEooqxWVZMI69972700 = OUKZMFFKnUEooqxWVZMI49427945;     OUKZMFFKnUEooqxWVZMI49427945 = OUKZMFFKnUEooqxWVZMI62380185;     OUKZMFFKnUEooqxWVZMI62380185 = OUKZMFFKnUEooqxWVZMI84747636;     OUKZMFFKnUEooqxWVZMI84747636 = OUKZMFFKnUEooqxWVZMI94204296;     OUKZMFFKnUEooqxWVZMI94204296 = OUKZMFFKnUEooqxWVZMI90554714;     OUKZMFFKnUEooqxWVZMI90554714 = OUKZMFFKnUEooqxWVZMI73153905;     OUKZMFFKnUEooqxWVZMI73153905 = OUKZMFFKnUEooqxWVZMI54074745;     OUKZMFFKnUEooqxWVZMI54074745 = OUKZMFFKnUEooqxWVZMI24861070;     OUKZMFFKnUEooqxWVZMI24861070 = OUKZMFFKnUEooqxWVZMI59224015;     OUKZMFFKnUEooqxWVZMI59224015 = OUKZMFFKnUEooqxWVZMI83243936;     OUKZMFFKnUEooqxWVZMI83243936 = OUKZMFFKnUEooqxWVZMI72918166;     OUKZMFFKnUEooqxWVZMI72918166 = OUKZMFFKnUEooqxWVZMI89930315;     OUKZMFFKnUEooqxWVZMI89930315 = OUKZMFFKnUEooqxWVZMI23745671;     OUKZMFFKnUEooqxWVZMI23745671 = OUKZMFFKnUEooqxWVZMI13504115;     OUKZMFFKnUEooqxWVZMI13504115 = OUKZMFFKnUEooqxWVZMI57926472;     OUKZMFFKnUEooqxWVZMI57926472 = OUKZMFFKnUEooqxWVZMI502775;     OUKZMFFKnUEooqxWVZMI502775 = OUKZMFFKnUEooqxWVZMI91844924;     OUKZMFFKnUEooqxWVZMI91844924 = OUKZMFFKnUEooqxWVZMI51850836;     OUKZMFFKnUEooqxWVZMI51850836 = OUKZMFFKnUEooqxWVZMI86853220;     OUKZMFFKnUEooqxWVZMI86853220 = OUKZMFFKnUEooqxWVZMI32195110;     OUKZMFFKnUEooqxWVZMI32195110 = OUKZMFFKnUEooqxWVZMI6161904;     OUKZMFFKnUEooqxWVZMI6161904 = OUKZMFFKnUEooqxWVZMI27232079;     OUKZMFFKnUEooqxWVZMI27232079 = OUKZMFFKnUEooqxWVZMI85636214;     OUKZMFFKnUEooqxWVZMI85636214 = OUKZMFFKnUEooqxWVZMI7038596;     OUKZMFFKnUEooqxWVZMI7038596 = OUKZMFFKnUEooqxWVZMI88583275;     OUKZMFFKnUEooqxWVZMI88583275 = OUKZMFFKnUEooqxWVZMI36513596;     OUKZMFFKnUEooqxWVZMI36513596 = OUKZMFFKnUEooqxWVZMI95868386;     OUKZMFFKnUEooqxWVZMI95868386 = OUKZMFFKnUEooqxWVZMI10720230;     OUKZMFFKnUEooqxWVZMI10720230 = OUKZMFFKnUEooqxWVZMI91041358;     OUKZMFFKnUEooqxWVZMI91041358 = OUKZMFFKnUEooqxWVZMI23720896;     OUKZMFFKnUEooqxWVZMI23720896 = OUKZMFFKnUEooqxWVZMI29910324;     OUKZMFFKnUEooqxWVZMI29910324 = OUKZMFFKnUEooqxWVZMI41739746;     OUKZMFFKnUEooqxWVZMI41739746 = OUKZMFFKnUEooqxWVZMI50629125;     OUKZMFFKnUEooqxWVZMI50629125 = OUKZMFFKnUEooqxWVZMI4070271;     OUKZMFFKnUEooqxWVZMI4070271 = OUKZMFFKnUEooqxWVZMI10194748;     OUKZMFFKnUEooqxWVZMI10194748 = OUKZMFFKnUEooqxWVZMI18684975;     OUKZMFFKnUEooqxWVZMI18684975 = OUKZMFFKnUEooqxWVZMI95653437;     OUKZMFFKnUEooqxWVZMI95653437 = OUKZMFFKnUEooqxWVZMI29950177;     OUKZMFFKnUEooqxWVZMI29950177 = OUKZMFFKnUEooqxWVZMI81633698;     OUKZMFFKnUEooqxWVZMI81633698 = OUKZMFFKnUEooqxWVZMI70546713;     OUKZMFFKnUEooqxWVZMI70546713 = OUKZMFFKnUEooqxWVZMI68734493;     OUKZMFFKnUEooqxWVZMI68734493 = OUKZMFFKnUEooqxWVZMI59517743;     OUKZMFFKnUEooqxWVZMI59517743 = OUKZMFFKnUEooqxWVZMI62181813;     OUKZMFFKnUEooqxWVZMI62181813 = OUKZMFFKnUEooqxWVZMI93325527;     OUKZMFFKnUEooqxWVZMI93325527 = OUKZMFFKnUEooqxWVZMI53474216;     OUKZMFFKnUEooqxWVZMI53474216 = OUKZMFFKnUEooqxWVZMI91856187;     OUKZMFFKnUEooqxWVZMI91856187 = OUKZMFFKnUEooqxWVZMI20499935;     OUKZMFFKnUEooqxWVZMI20499935 = OUKZMFFKnUEooqxWVZMI50331096;     OUKZMFFKnUEooqxWVZMI50331096 = OUKZMFFKnUEooqxWVZMI23691704;     OUKZMFFKnUEooqxWVZMI23691704 = OUKZMFFKnUEooqxWVZMI70521286;     OUKZMFFKnUEooqxWVZMI70521286 = OUKZMFFKnUEooqxWVZMI34530406;     OUKZMFFKnUEooqxWVZMI34530406 = OUKZMFFKnUEooqxWVZMI19430798;     OUKZMFFKnUEooqxWVZMI19430798 = OUKZMFFKnUEooqxWVZMI26702909;     OUKZMFFKnUEooqxWVZMI26702909 = OUKZMFFKnUEooqxWVZMI83083494;     OUKZMFFKnUEooqxWVZMI83083494 = OUKZMFFKnUEooqxWVZMI63810796;     OUKZMFFKnUEooqxWVZMI63810796 = OUKZMFFKnUEooqxWVZMI22195867;     OUKZMFFKnUEooqxWVZMI22195867 = OUKZMFFKnUEooqxWVZMI76743970;     OUKZMFFKnUEooqxWVZMI76743970 = OUKZMFFKnUEooqxWVZMI77709041;     OUKZMFFKnUEooqxWVZMI77709041 = OUKZMFFKnUEooqxWVZMI5621021;     OUKZMFFKnUEooqxWVZMI5621021 = OUKZMFFKnUEooqxWVZMI54041118;     OUKZMFFKnUEooqxWVZMI54041118 = OUKZMFFKnUEooqxWVZMI77285518;     OUKZMFFKnUEooqxWVZMI77285518 = OUKZMFFKnUEooqxWVZMI43354516;     OUKZMFFKnUEooqxWVZMI43354516 = OUKZMFFKnUEooqxWVZMI33819711;     OUKZMFFKnUEooqxWVZMI33819711 = OUKZMFFKnUEooqxWVZMI35503120;     OUKZMFFKnUEooqxWVZMI35503120 = OUKZMFFKnUEooqxWVZMI53333612;     OUKZMFFKnUEooqxWVZMI53333612 = OUKZMFFKnUEooqxWVZMI31178421;     OUKZMFFKnUEooqxWVZMI31178421 = OUKZMFFKnUEooqxWVZMI48637907;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gqNOorwpGQWzCURMOVQv18134974() {     double LRbWcnNaNVMyUofFLKEQ74223183 = 7279999;    double LRbWcnNaNVMyUofFLKEQ58602821 = 60951630;    double LRbWcnNaNVMyUofFLKEQ66310154 = -766058509;    double LRbWcnNaNVMyUofFLKEQ16091709 = -575666422;    double LRbWcnNaNVMyUofFLKEQ50101206 = -993579578;    double LRbWcnNaNVMyUofFLKEQ76712874 = -474138789;    double LRbWcnNaNVMyUofFLKEQ97659248 = -809882695;    double LRbWcnNaNVMyUofFLKEQ77075543 = -101310301;    double LRbWcnNaNVMyUofFLKEQ75425518 = -798157263;    double LRbWcnNaNVMyUofFLKEQ20350340 = -821475758;    double LRbWcnNaNVMyUofFLKEQ69629429 = -594874965;    double LRbWcnNaNVMyUofFLKEQ1225386 = -428291346;    double LRbWcnNaNVMyUofFLKEQ50623748 = -175040197;    double LRbWcnNaNVMyUofFLKEQ51182410 = 89473671;    double LRbWcnNaNVMyUofFLKEQ37334047 = -712395691;    double LRbWcnNaNVMyUofFLKEQ88122116 = -935281284;    double LRbWcnNaNVMyUofFLKEQ23952590 = -831006009;    double LRbWcnNaNVMyUofFLKEQ93304316 = -190556565;    double LRbWcnNaNVMyUofFLKEQ5165340 = -801878474;    double LRbWcnNaNVMyUofFLKEQ83970705 = -859164583;    double LRbWcnNaNVMyUofFLKEQ85388078 = -196294580;    double LRbWcnNaNVMyUofFLKEQ22908935 = -676730997;    double LRbWcnNaNVMyUofFLKEQ80333261 = -601237889;    double LRbWcnNaNVMyUofFLKEQ28106426 = -990861284;    double LRbWcnNaNVMyUofFLKEQ36689643 = -106145104;    double LRbWcnNaNVMyUofFLKEQ73734742 = -367062184;    double LRbWcnNaNVMyUofFLKEQ46223120 = -298192416;    double LRbWcnNaNVMyUofFLKEQ45516975 = -926143856;    double LRbWcnNaNVMyUofFLKEQ78901273 = -210420707;    double LRbWcnNaNVMyUofFLKEQ79697821 = -629729805;    double LRbWcnNaNVMyUofFLKEQ93328267 = -495968677;    double LRbWcnNaNVMyUofFLKEQ50746062 = -980330209;    double LRbWcnNaNVMyUofFLKEQ36465656 = 92089925;    double LRbWcnNaNVMyUofFLKEQ23559771 = -810891221;    double LRbWcnNaNVMyUofFLKEQ50832933 = -23330948;    double LRbWcnNaNVMyUofFLKEQ67172035 = -408524839;    double LRbWcnNaNVMyUofFLKEQ34393022 = -963272911;    double LRbWcnNaNVMyUofFLKEQ69776983 = 80237440;    double LRbWcnNaNVMyUofFLKEQ31478293 = -735946883;    double LRbWcnNaNVMyUofFLKEQ81937579 = -670978070;    double LRbWcnNaNVMyUofFLKEQ1226026 = 66290068;    double LRbWcnNaNVMyUofFLKEQ68870660 = -294678082;    double LRbWcnNaNVMyUofFLKEQ8424235 = -812754726;    double LRbWcnNaNVMyUofFLKEQ54037215 = -179582609;    double LRbWcnNaNVMyUofFLKEQ64899043 = -310419509;    double LRbWcnNaNVMyUofFLKEQ56330638 = -593781422;    double LRbWcnNaNVMyUofFLKEQ23514125 = -766877580;    double LRbWcnNaNVMyUofFLKEQ21616885 = 4650418;    double LRbWcnNaNVMyUofFLKEQ8737792 = -681374192;    double LRbWcnNaNVMyUofFLKEQ54312769 = -498792645;    double LRbWcnNaNVMyUofFLKEQ1981172 = -629072718;    double LRbWcnNaNVMyUofFLKEQ79109415 = 68216847;    double LRbWcnNaNVMyUofFLKEQ4926824 = 41540384;    double LRbWcnNaNVMyUofFLKEQ1094019 = -843668839;    double LRbWcnNaNVMyUofFLKEQ32746895 = -197532991;    double LRbWcnNaNVMyUofFLKEQ51314249 = -315989005;    double LRbWcnNaNVMyUofFLKEQ78269560 = -337810481;    double LRbWcnNaNVMyUofFLKEQ38203729 = -775197226;    double LRbWcnNaNVMyUofFLKEQ79402066 = -369521319;    double LRbWcnNaNVMyUofFLKEQ76366464 = -526517394;    double LRbWcnNaNVMyUofFLKEQ30489754 = -75946374;    double LRbWcnNaNVMyUofFLKEQ52142274 = -883738840;    double LRbWcnNaNVMyUofFLKEQ98174270 = -890889594;    double LRbWcnNaNVMyUofFLKEQ95727697 = -68427459;    double LRbWcnNaNVMyUofFLKEQ27022072 = -225507082;    double LRbWcnNaNVMyUofFLKEQ18883367 = -614544757;    double LRbWcnNaNVMyUofFLKEQ64759729 = -420381272;    double LRbWcnNaNVMyUofFLKEQ27063977 = -364148976;    double LRbWcnNaNVMyUofFLKEQ349477 = -887195382;    double LRbWcnNaNVMyUofFLKEQ70162012 = -203870852;    double LRbWcnNaNVMyUofFLKEQ53729095 = -972008373;    double LRbWcnNaNVMyUofFLKEQ54175607 = -811243450;    double LRbWcnNaNVMyUofFLKEQ61826024 = -454609682;    double LRbWcnNaNVMyUofFLKEQ23227761 = -30900405;    double LRbWcnNaNVMyUofFLKEQ82744679 = -825454652;    double LRbWcnNaNVMyUofFLKEQ16517419 = -901616498;    double LRbWcnNaNVMyUofFLKEQ14484701 = -863976272;    double LRbWcnNaNVMyUofFLKEQ26296046 = -321655281;    double LRbWcnNaNVMyUofFLKEQ63207383 = -580441775;    double LRbWcnNaNVMyUofFLKEQ80359004 = -512363683;    double LRbWcnNaNVMyUofFLKEQ50220617 = -600184604;    double LRbWcnNaNVMyUofFLKEQ24606235 = -202842834;    double LRbWcnNaNVMyUofFLKEQ36779184 = -144769664;    double LRbWcnNaNVMyUofFLKEQ24588504 = -711628062;    double LRbWcnNaNVMyUofFLKEQ77716649 = 99342913;    double LRbWcnNaNVMyUofFLKEQ14218853 = -464185524;    double LRbWcnNaNVMyUofFLKEQ45819238 = -921870593;    double LRbWcnNaNVMyUofFLKEQ35371638 = -64241237;    double LRbWcnNaNVMyUofFLKEQ90812875 = -513358230;    double LRbWcnNaNVMyUofFLKEQ99518684 = -707341943;    double LRbWcnNaNVMyUofFLKEQ88902475 = 29285642;    double LRbWcnNaNVMyUofFLKEQ96189293 = -88075685;    double LRbWcnNaNVMyUofFLKEQ90374917 = -550241241;    double LRbWcnNaNVMyUofFLKEQ55111829 = -109429489;    double LRbWcnNaNVMyUofFLKEQ51447825 = -495031696;    double LRbWcnNaNVMyUofFLKEQ49083752 = -49971092;    double LRbWcnNaNVMyUofFLKEQ70696389 = -403788488;    double LRbWcnNaNVMyUofFLKEQ12696538 = -644327267;    double LRbWcnNaNVMyUofFLKEQ27015144 = -954075528;    double LRbWcnNaNVMyUofFLKEQ46015676 = 7279999;     LRbWcnNaNVMyUofFLKEQ74223183 = LRbWcnNaNVMyUofFLKEQ58602821;     LRbWcnNaNVMyUofFLKEQ58602821 = LRbWcnNaNVMyUofFLKEQ66310154;     LRbWcnNaNVMyUofFLKEQ66310154 = LRbWcnNaNVMyUofFLKEQ16091709;     LRbWcnNaNVMyUofFLKEQ16091709 = LRbWcnNaNVMyUofFLKEQ50101206;     LRbWcnNaNVMyUofFLKEQ50101206 = LRbWcnNaNVMyUofFLKEQ76712874;     LRbWcnNaNVMyUofFLKEQ76712874 = LRbWcnNaNVMyUofFLKEQ97659248;     LRbWcnNaNVMyUofFLKEQ97659248 = LRbWcnNaNVMyUofFLKEQ77075543;     LRbWcnNaNVMyUofFLKEQ77075543 = LRbWcnNaNVMyUofFLKEQ75425518;     LRbWcnNaNVMyUofFLKEQ75425518 = LRbWcnNaNVMyUofFLKEQ20350340;     LRbWcnNaNVMyUofFLKEQ20350340 = LRbWcnNaNVMyUofFLKEQ69629429;     LRbWcnNaNVMyUofFLKEQ69629429 = LRbWcnNaNVMyUofFLKEQ1225386;     LRbWcnNaNVMyUofFLKEQ1225386 = LRbWcnNaNVMyUofFLKEQ50623748;     LRbWcnNaNVMyUofFLKEQ50623748 = LRbWcnNaNVMyUofFLKEQ51182410;     LRbWcnNaNVMyUofFLKEQ51182410 = LRbWcnNaNVMyUofFLKEQ37334047;     LRbWcnNaNVMyUofFLKEQ37334047 = LRbWcnNaNVMyUofFLKEQ88122116;     LRbWcnNaNVMyUofFLKEQ88122116 = LRbWcnNaNVMyUofFLKEQ23952590;     LRbWcnNaNVMyUofFLKEQ23952590 = LRbWcnNaNVMyUofFLKEQ93304316;     LRbWcnNaNVMyUofFLKEQ93304316 = LRbWcnNaNVMyUofFLKEQ5165340;     LRbWcnNaNVMyUofFLKEQ5165340 = LRbWcnNaNVMyUofFLKEQ83970705;     LRbWcnNaNVMyUofFLKEQ83970705 = LRbWcnNaNVMyUofFLKEQ85388078;     LRbWcnNaNVMyUofFLKEQ85388078 = LRbWcnNaNVMyUofFLKEQ22908935;     LRbWcnNaNVMyUofFLKEQ22908935 = LRbWcnNaNVMyUofFLKEQ80333261;     LRbWcnNaNVMyUofFLKEQ80333261 = LRbWcnNaNVMyUofFLKEQ28106426;     LRbWcnNaNVMyUofFLKEQ28106426 = LRbWcnNaNVMyUofFLKEQ36689643;     LRbWcnNaNVMyUofFLKEQ36689643 = LRbWcnNaNVMyUofFLKEQ73734742;     LRbWcnNaNVMyUofFLKEQ73734742 = LRbWcnNaNVMyUofFLKEQ46223120;     LRbWcnNaNVMyUofFLKEQ46223120 = LRbWcnNaNVMyUofFLKEQ45516975;     LRbWcnNaNVMyUofFLKEQ45516975 = LRbWcnNaNVMyUofFLKEQ78901273;     LRbWcnNaNVMyUofFLKEQ78901273 = LRbWcnNaNVMyUofFLKEQ79697821;     LRbWcnNaNVMyUofFLKEQ79697821 = LRbWcnNaNVMyUofFLKEQ93328267;     LRbWcnNaNVMyUofFLKEQ93328267 = LRbWcnNaNVMyUofFLKEQ50746062;     LRbWcnNaNVMyUofFLKEQ50746062 = LRbWcnNaNVMyUofFLKEQ36465656;     LRbWcnNaNVMyUofFLKEQ36465656 = LRbWcnNaNVMyUofFLKEQ23559771;     LRbWcnNaNVMyUofFLKEQ23559771 = LRbWcnNaNVMyUofFLKEQ50832933;     LRbWcnNaNVMyUofFLKEQ50832933 = LRbWcnNaNVMyUofFLKEQ67172035;     LRbWcnNaNVMyUofFLKEQ67172035 = LRbWcnNaNVMyUofFLKEQ34393022;     LRbWcnNaNVMyUofFLKEQ34393022 = LRbWcnNaNVMyUofFLKEQ69776983;     LRbWcnNaNVMyUofFLKEQ69776983 = LRbWcnNaNVMyUofFLKEQ31478293;     LRbWcnNaNVMyUofFLKEQ31478293 = LRbWcnNaNVMyUofFLKEQ81937579;     LRbWcnNaNVMyUofFLKEQ81937579 = LRbWcnNaNVMyUofFLKEQ1226026;     LRbWcnNaNVMyUofFLKEQ1226026 = LRbWcnNaNVMyUofFLKEQ68870660;     LRbWcnNaNVMyUofFLKEQ68870660 = LRbWcnNaNVMyUofFLKEQ8424235;     LRbWcnNaNVMyUofFLKEQ8424235 = LRbWcnNaNVMyUofFLKEQ54037215;     LRbWcnNaNVMyUofFLKEQ54037215 = LRbWcnNaNVMyUofFLKEQ64899043;     LRbWcnNaNVMyUofFLKEQ64899043 = LRbWcnNaNVMyUofFLKEQ56330638;     LRbWcnNaNVMyUofFLKEQ56330638 = LRbWcnNaNVMyUofFLKEQ23514125;     LRbWcnNaNVMyUofFLKEQ23514125 = LRbWcnNaNVMyUofFLKEQ21616885;     LRbWcnNaNVMyUofFLKEQ21616885 = LRbWcnNaNVMyUofFLKEQ8737792;     LRbWcnNaNVMyUofFLKEQ8737792 = LRbWcnNaNVMyUofFLKEQ54312769;     LRbWcnNaNVMyUofFLKEQ54312769 = LRbWcnNaNVMyUofFLKEQ1981172;     LRbWcnNaNVMyUofFLKEQ1981172 = LRbWcnNaNVMyUofFLKEQ79109415;     LRbWcnNaNVMyUofFLKEQ79109415 = LRbWcnNaNVMyUofFLKEQ4926824;     LRbWcnNaNVMyUofFLKEQ4926824 = LRbWcnNaNVMyUofFLKEQ1094019;     LRbWcnNaNVMyUofFLKEQ1094019 = LRbWcnNaNVMyUofFLKEQ32746895;     LRbWcnNaNVMyUofFLKEQ32746895 = LRbWcnNaNVMyUofFLKEQ51314249;     LRbWcnNaNVMyUofFLKEQ51314249 = LRbWcnNaNVMyUofFLKEQ78269560;     LRbWcnNaNVMyUofFLKEQ78269560 = LRbWcnNaNVMyUofFLKEQ38203729;     LRbWcnNaNVMyUofFLKEQ38203729 = LRbWcnNaNVMyUofFLKEQ79402066;     LRbWcnNaNVMyUofFLKEQ79402066 = LRbWcnNaNVMyUofFLKEQ76366464;     LRbWcnNaNVMyUofFLKEQ76366464 = LRbWcnNaNVMyUofFLKEQ30489754;     LRbWcnNaNVMyUofFLKEQ30489754 = LRbWcnNaNVMyUofFLKEQ52142274;     LRbWcnNaNVMyUofFLKEQ52142274 = LRbWcnNaNVMyUofFLKEQ98174270;     LRbWcnNaNVMyUofFLKEQ98174270 = LRbWcnNaNVMyUofFLKEQ95727697;     LRbWcnNaNVMyUofFLKEQ95727697 = LRbWcnNaNVMyUofFLKEQ27022072;     LRbWcnNaNVMyUofFLKEQ27022072 = LRbWcnNaNVMyUofFLKEQ18883367;     LRbWcnNaNVMyUofFLKEQ18883367 = LRbWcnNaNVMyUofFLKEQ64759729;     LRbWcnNaNVMyUofFLKEQ64759729 = LRbWcnNaNVMyUofFLKEQ27063977;     LRbWcnNaNVMyUofFLKEQ27063977 = LRbWcnNaNVMyUofFLKEQ349477;     LRbWcnNaNVMyUofFLKEQ349477 = LRbWcnNaNVMyUofFLKEQ70162012;     LRbWcnNaNVMyUofFLKEQ70162012 = LRbWcnNaNVMyUofFLKEQ53729095;     LRbWcnNaNVMyUofFLKEQ53729095 = LRbWcnNaNVMyUofFLKEQ54175607;     LRbWcnNaNVMyUofFLKEQ54175607 = LRbWcnNaNVMyUofFLKEQ61826024;     LRbWcnNaNVMyUofFLKEQ61826024 = LRbWcnNaNVMyUofFLKEQ23227761;     LRbWcnNaNVMyUofFLKEQ23227761 = LRbWcnNaNVMyUofFLKEQ82744679;     LRbWcnNaNVMyUofFLKEQ82744679 = LRbWcnNaNVMyUofFLKEQ16517419;     LRbWcnNaNVMyUofFLKEQ16517419 = LRbWcnNaNVMyUofFLKEQ14484701;     LRbWcnNaNVMyUofFLKEQ14484701 = LRbWcnNaNVMyUofFLKEQ26296046;     LRbWcnNaNVMyUofFLKEQ26296046 = LRbWcnNaNVMyUofFLKEQ63207383;     LRbWcnNaNVMyUofFLKEQ63207383 = LRbWcnNaNVMyUofFLKEQ80359004;     LRbWcnNaNVMyUofFLKEQ80359004 = LRbWcnNaNVMyUofFLKEQ50220617;     LRbWcnNaNVMyUofFLKEQ50220617 = LRbWcnNaNVMyUofFLKEQ24606235;     LRbWcnNaNVMyUofFLKEQ24606235 = LRbWcnNaNVMyUofFLKEQ36779184;     LRbWcnNaNVMyUofFLKEQ36779184 = LRbWcnNaNVMyUofFLKEQ24588504;     LRbWcnNaNVMyUofFLKEQ24588504 = LRbWcnNaNVMyUofFLKEQ77716649;     LRbWcnNaNVMyUofFLKEQ77716649 = LRbWcnNaNVMyUofFLKEQ14218853;     LRbWcnNaNVMyUofFLKEQ14218853 = LRbWcnNaNVMyUofFLKEQ45819238;     LRbWcnNaNVMyUofFLKEQ45819238 = LRbWcnNaNVMyUofFLKEQ35371638;     LRbWcnNaNVMyUofFLKEQ35371638 = LRbWcnNaNVMyUofFLKEQ90812875;     LRbWcnNaNVMyUofFLKEQ90812875 = LRbWcnNaNVMyUofFLKEQ99518684;     LRbWcnNaNVMyUofFLKEQ99518684 = LRbWcnNaNVMyUofFLKEQ88902475;     LRbWcnNaNVMyUofFLKEQ88902475 = LRbWcnNaNVMyUofFLKEQ96189293;     LRbWcnNaNVMyUofFLKEQ96189293 = LRbWcnNaNVMyUofFLKEQ90374917;     LRbWcnNaNVMyUofFLKEQ90374917 = LRbWcnNaNVMyUofFLKEQ55111829;     LRbWcnNaNVMyUofFLKEQ55111829 = LRbWcnNaNVMyUofFLKEQ51447825;     LRbWcnNaNVMyUofFLKEQ51447825 = LRbWcnNaNVMyUofFLKEQ49083752;     LRbWcnNaNVMyUofFLKEQ49083752 = LRbWcnNaNVMyUofFLKEQ70696389;     LRbWcnNaNVMyUofFLKEQ70696389 = LRbWcnNaNVMyUofFLKEQ12696538;     LRbWcnNaNVMyUofFLKEQ12696538 = LRbWcnNaNVMyUofFLKEQ27015144;     LRbWcnNaNVMyUofFLKEQ27015144 = LRbWcnNaNVMyUofFLKEQ46015676;     LRbWcnNaNVMyUofFLKEQ46015676 = LRbWcnNaNVMyUofFLKEQ74223183;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GbqpzowvhdoRVhfsTDxs76045658() {     double jtmLkmCzsEtgEnyuUAnc67541180 = -814629481;    double jtmLkmCzsEtgEnyuUAnc92349845 = -663504388;    double jtmLkmCzsEtgEnyuUAnc67225962 = -422789276;    double jtmLkmCzsEtgEnyuUAnc5385386 = -583294436;    double jtmLkmCzsEtgEnyuUAnc43165795 = -859123627;    double jtmLkmCzsEtgEnyuUAnc94430870 = -360577334;    double jtmLkmCzsEtgEnyuUAnc74035128 = -912248536;    double jtmLkmCzsEtgEnyuUAnc38544423 = -452020592;    double jtmLkmCzsEtgEnyuUAnc10143532 = -31723976;    double jtmLkmCzsEtgEnyuUAnc48640321 = -619977345;    double jtmLkmCzsEtgEnyuUAnc42893548 = -276570680;    double jtmLkmCzsEtgEnyuUAnc29681277 = -936321458;    double jtmLkmCzsEtgEnyuUAnc30395392 = -754465577;    double jtmLkmCzsEtgEnyuUAnc92390448 = -800655801;    double jtmLkmCzsEtgEnyuUAnc114140 = -321297730;    double jtmLkmCzsEtgEnyuUAnc50331536 = 77019261;    double jtmLkmCzsEtgEnyuUAnc73742294 = -323005795;    double jtmLkmCzsEtgEnyuUAnc52800050 = 4987851;    double jtmLkmCzsEtgEnyuUAnc6691091 = -334142909;    double jtmLkmCzsEtgEnyuUAnc6013900 = -740872266;    double jtmLkmCzsEtgEnyuUAnc53106063 = -81398518;    double jtmLkmCzsEtgEnyuUAnc36974671 = -334786131;    double jtmLkmCzsEtgEnyuUAnc50920551 = -234667166;    double jtmLkmCzsEtgEnyuUAnc54891886 = -614662304;    double jtmLkmCzsEtgEnyuUAnc31380413 = -22286919;    double jtmLkmCzsEtgEnyuUAnc50285926 = -371698165;    double jtmLkmCzsEtgEnyuUAnc73710525 = -166747764;    double jtmLkmCzsEtgEnyuUAnc62078225 = -936208681;    double jtmLkmCzsEtgEnyuUAnc53773369 = -410726801;    double jtmLkmCzsEtgEnyuUAnc47742082 = -168177265;    double jtmLkmCzsEtgEnyuUAnc7960179 = -907059334;    double jtmLkmCzsEtgEnyuUAnc14485575 = -394700869;    double jtmLkmCzsEtgEnyuUAnc71035502 = -328557093;    double jtmLkmCzsEtgEnyuUAnc41879637 = -411042233;    double jtmLkmCzsEtgEnyuUAnc93963129 = -570660022;    double jtmLkmCzsEtgEnyuUAnc26330213 = -993519335;    double jtmLkmCzsEtgEnyuUAnc2773632 = -226306187;    double jtmLkmCzsEtgEnyuUAnc85998531 = -862173836;    double jtmLkmCzsEtgEnyuUAnc69940354 = -113382635;    double jtmLkmCzsEtgEnyuUAnc86920572 = -518863819;    double jtmLkmCzsEtgEnyuUAnc21175277 = -899397629;    double jtmLkmCzsEtgEnyuUAnc64713983 = 45906810;    double jtmLkmCzsEtgEnyuUAnc72174463 = -888386373;    double jtmLkmCzsEtgEnyuUAnc15280215 = -71682192;    double jtmLkmCzsEtgEnyuUAnc64879122 = 73933494;    double jtmLkmCzsEtgEnyuUAnc75497640 = -129768440;    double jtmLkmCzsEtgEnyuUAnc19106265 = -250699310;    double jtmLkmCzsEtgEnyuUAnc50901021 = 32584953;    double jtmLkmCzsEtgEnyuUAnc11666250 = -831036928;    double jtmLkmCzsEtgEnyuUAnc60364847 = -248816763;    double jtmLkmCzsEtgEnyuUAnc45873096 = -475592095;    double jtmLkmCzsEtgEnyuUAnc90837102 = -25615632;    double jtmLkmCzsEtgEnyuUAnc69586828 = -458353938;    double jtmLkmCzsEtgEnyuUAnc72863715 = -438526945;    double jtmLkmCzsEtgEnyuUAnc22939595 = -417418843;    double jtmLkmCzsEtgEnyuUAnc30566510 = -379843350;    double jtmLkmCzsEtgEnyuUAnc41429295 = -328837223;    double jtmLkmCzsEtgEnyuUAnc12334076 = -808126973;    double jtmLkmCzsEtgEnyuUAnc74004972 = -461007518;    double jtmLkmCzsEtgEnyuUAnc92879868 = -387425463;    double jtmLkmCzsEtgEnyuUAnc20720346 = -93829570;    double jtmLkmCzsEtgEnyuUAnc11956904 = -976039855;    double jtmLkmCzsEtgEnyuUAnc84771053 = 58706209;    double jtmLkmCzsEtgEnyuUAnc62401449 = -863546712;    double jtmLkmCzsEtgEnyuUAnc40680143 = -712918012;    double jtmLkmCzsEtgEnyuUAnc28407973 = -881869812;    double jtmLkmCzsEtgEnyuUAnc58645775 = -507764365;    double jtmLkmCzsEtgEnyuUAnc88515754 = -243423345;    double jtmLkmCzsEtgEnyuUAnc98427319 = -129995780;    double jtmLkmCzsEtgEnyuUAnc73783926 = -327778396;    double jtmLkmCzsEtgEnyuUAnc47557905 = -696674553;    double jtmLkmCzsEtgEnyuUAnc87743762 = -460831959;    double jtmLkmCzsEtgEnyuUAnc82859696 = -881629514;    double jtmLkmCzsEtgEnyuUAnc19770518 = -815279091;    double jtmLkmCzsEtgEnyuUAnc84838622 = -841474638;    double jtmLkmCzsEtgEnyuUAnc88392080 = -27305328;    double jtmLkmCzsEtgEnyuUAnc64800208 = -446399759;    double jtmLkmCzsEtgEnyuUAnc35640336 = -62984975;    double jtmLkmCzsEtgEnyuUAnc90012764 = -588595799;    double jtmLkmCzsEtgEnyuUAnc55882772 = -892518479;    double jtmLkmCzsEtgEnyuUAnc31179662 = -20998855;    double jtmLkmCzsEtgEnyuUAnc22809504 = -99332718;    double jtmLkmCzsEtgEnyuUAnc50411975 = -5171754;    double jtmLkmCzsEtgEnyuUAnc93408521 = -61910039;    double jtmLkmCzsEtgEnyuUAnc1868987 = -692585170;    double jtmLkmCzsEtgEnyuUAnc17123076 = -781443702;    double jtmLkmCzsEtgEnyuUAnc44898747 = -936346932;    double jtmLkmCzsEtgEnyuUAnc98171786 = -890030149;    double jtmLkmCzsEtgEnyuUAnc18940043 = -993623390;    double jtmLkmCzsEtgEnyuUAnc63396619 = -90816673;    double jtmLkmCzsEtgEnyuUAnc84900918 = -564682112;    double jtmLkmCzsEtgEnyuUAnc90439555 = -418179215;    double jtmLkmCzsEtgEnyuUAnc11993560 = -301166319;    double jtmLkmCzsEtgEnyuUAnc77060485 = -725957173;    double jtmLkmCzsEtgEnyuUAnc66200227 = -325034249;    double jtmLkmCzsEtgEnyuUAnc9218374 = -923357775;    double jtmLkmCzsEtgEnyuUAnc79942929 = 87200600;    double jtmLkmCzsEtgEnyuUAnc9773014 = 75160338;    double jtmLkmCzsEtgEnyuUAnc74600072 = -358764180;    double jtmLkmCzsEtgEnyuUAnc36471149 = -814629481;     jtmLkmCzsEtgEnyuUAnc67541180 = jtmLkmCzsEtgEnyuUAnc92349845;     jtmLkmCzsEtgEnyuUAnc92349845 = jtmLkmCzsEtgEnyuUAnc67225962;     jtmLkmCzsEtgEnyuUAnc67225962 = jtmLkmCzsEtgEnyuUAnc5385386;     jtmLkmCzsEtgEnyuUAnc5385386 = jtmLkmCzsEtgEnyuUAnc43165795;     jtmLkmCzsEtgEnyuUAnc43165795 = jtmLkmCzsEtgEnyuUAnc94430870;     jtmLkmCzsEtgEnyuUAnc94430870 = jtmLkmCzsEtgEnyuUAnc74035128;     jtmLkmCzsEtgEnyuUAnc74035128 = jtmLkmCzsEtgEnyuUAnc38544423;     jtmLkmCzsEtgEnyuUAnc38544423 = jtmLkmCzsEtgEnyuUAnc10143532;     jtmLkmCzsEtgEnyuUAnc10143532 = jtmLkmCzsEtgEnyuUAnc48640321;     jtmLkmCzsEtgEnyuUAnc48640321 = jtmLkmCzsEtgEnyuUAnc42893548;     jtmLkmCzsEtgEnyuUAnc42893548 = jtmLkmCzsEtgEnyuUAnc29681277;     jtmLkmCzsEtgEnyuUAnc29681277 = jtmLkmCzsEtgEnyuUAnc30395392;     jtmLkmCzsEtgEnyuUAnc30395392 = jtmLkmCzsEtgEnyuUAnc92390448;     jtmLkmCzsEtgEnyuUAnc92390448 = jtmLkmCzsEtgEnyuUAnc114140;     jtmLkmCzsEtgEnyuUAnc114140 = jtmLkmCzsEtgEnyuUAnc50331536;     jtmLkmCzsEtgEnyuUAnc50331536 = jtmLkmCzsEtgEnyuUAnc73742294;     jtmLkmCzsEtgEnyuUAnc73742294 = jtmLkmCzsEtgEnyuUAnc52800050;     jtmLkmCzsEtgEnyuUAnc52800050 = jtmLkmCzsEtgEnyuUAnc6691091;     jtmLkmCzsEtgEnyuUAnc6691091 = jtmLkmCzsEtgEnyuUAnc6013900;     jtmLkmCzsEtgEnyuUAnc6013900 = jtmLkmCzsEtgEnyuUAnc53106063;     jtmLkmCzsEtgEnyuUAnc53106063 = jtmLkmCzsEtgEnyuUAnc36974671;     jtmLkmCzsEtgEnyuUAnc36974671 = jtmLkmCzsEtgEnyuUAnc50920551;     jtmLkmCzsEtgEnyuUAnc50920551 = jtmLkmCzsEtgEnyuUAnc54891886;     jtmLkmCzsEtgEnyuUAnc54891886 = jtmLkmCzsEtgEnyuUAnc31380413;     jtmLkmCzsEtgEnyuUAnc31380413 = jtmLkmCzsEtgEnyuUAnc50285926;     jtmLkmCzsEtgEnyuUAnc50285926 = jtmLkmCzsEtgEnyuUAnc73710525;     jtmLkmCzsEtgEnyuUAnc73710525 = jtmLkmCzsEtgEnyuUAnc62078225;     jtmLkmCzsEtgEnyuUAnc62078225 = jtmLkmCzsEtgEnyuUAnc53773369;     jtmLkmCzsEtgEnyuUAnc53773369 = jtmLkmCzsEtgEnyuUAnc47742082;     jtmLkmCzsEtgEnyuUAnc47742082 = jtmLkmCzsEtgEnyuUAnc7960179;     jtmLkmCzsEtgEnyuUAnc7960179 = jtmLkmCzsEtgEnyuUAnc14485575;     jtmLkmCzsEtgEnyuUAnc14485575 = jtmLkmCzsEtgEnyuUAnc71035502;     jtmLkmCzsEtgEnyuUAnc71035502 = jtmLkmCzsEtgEnyuUAnc41879637;     jtmLkmCzsEtgEnyuUAnc41879637 = jtmLkmCzsEtgEnyuUAnc93963129;     jtmLkmCzsEtgEnyuUAnc93963129 = jtmLkmCzsEtgEnyuUAnc26330213;     jtmLkmCzsEtgEnyuUAnc26330213 = jtmLkmCzsEtgEnyuUAnc2773632;     jtmLkmCzsEtgEnyuUAnc2773632 = jtmLkmCzsEtgEnyuUAnc85998531;     jtmLkmCzsEtgEnyuUAnc85998531 = jtmLkmCzsEtgEnyuUAnc69940354;     jtmLkmCzsEtgEnyuUAnc69940354 = jtmLkmCzsEtgEnyuUAnc86920572;     jtmLkmCzsEtgEnyuUAnc86920572 = jtmLkmCzsEtgEnyuUAnc21175277;     jtmLkmCzsEtgEnyuUAnc21175277 = jtmLkmCzsEtgEnyuUAnc64713983;     jtmLkmCzsEtgEnyuUAnc64713983 = jtmLkmCzsEtgEnyuUAnc72174463;     jtmLkmCzsEtgEnyuUAnc72174463 = jtmLkmCzsEtgEnyuUAnc15280215;     jtmLkmCzsEtgEnyuUAnc15280215 = jtmLkmCzsEtgEnyuUAnc64879122;     jtmLkmCzsEtgEnyuUAnc64879122 = jtmLkmCzsEtgEnyuUAnc75497640;     jtmLkmCzsEtgEnyuUAnc75497640 = jtmLkmCzsEtgEnyuUAnc19106265;     jtmLkmCzsEtgEnyuUAnc19106265 = jtmLkmCzsEtgEnyuUAnc50901021;     jtmLkmCzsEtgEnyuUAnc50901021 = jtmLkmCzsEtgEnyuUAnc11666250;     jtmLkmCzsEtgEnyuUAnc11666250 = jtmLkmCzsEtgEnyuUAnc60364847;     jtmLkmCzsEtgEnyuUAnc60364847 = jtmLkmCzsEtgEnyuUAnc45873096;     jtmLkmCzsEtgEnyuUAnc45873096 = jtmLkmCzsEtgEnyuUAnc90837102;     jtmLkmCzsEtgEnyuUAnc90837102 = jtmLkmCzsEtgEnyuUAnc69586828;     jtmLkmCzsEtgEnyuUAnc69586828 = jtmLkmCzsEtgEnyuUAnc72863715;     jtmLkmCzsEtgEnyuUAnc72863715 = jtmLkmCzsEtgEnyuUAnc22939595;     jtmLkmCzsEtgEnyuUAnc22939595 = jtmLkmCzsEtgEnyuUAnc30566510;     jtmLkmCzsEtgEnyuUAnc30566510 = jtmLkmCzsEtgEnyuUAnc41429295;     jtmLkmCzsEtgEnyuUAnc41429295 = jtmLkmCzsEtgEnyuUAnc12334076;     jtmLkmCzsEtgEnyuUAnc12334076 = jtmLkmCzsEtgEnyuUAnc74004972;     jtmLkmCzsEtgEnyuUAnc74004972 = jtmLkmCzsEtgEnyuUAnc92879868;     jtmLkmCzsEtgEnyuUAnc92879868 = jtmLkmCzsEtgEnyuUAnc20720346;     jtmLkmCzsEtgEnyuUAnc20720346 = jtmLkmCzsEtgEnyuUAnc11956904;     jtmLkmCzsEtgEnyuUAnc11956904 = jtmLkmCzsEtgEnyuUAnc84771053;     jtmLkmCzsEtgEnyuUAnc84771053 = jtmLkmCzsEtgEnyuUAnc62401449;     jtmLkmCzsEtgEnyuUAnc62401449 = jtmLkmCzsEtgEnyuUAnc40680143;     jtmLkmCzsEtgEnyuUAnc40680143 = jtmLkmCzsEtgEnyuUAnc28407973;     jtmLkmCzsEtgEnyuUAnc28407973 = jtmLkmCzsEtgEnyuUAnc58645775;     jtmLkmCzsEtgEnyuUAnc58645775 = jtmLkmCzsEtgEnyuUAnc88515754;     jtmLkmCzsEtgEnyuUAnc88515754 = jtmLkmCzsEtgEnyuUAnc98427319;     jtmLkmCzsEtgEnyuUAnc98427319 = jtmLkmCzsEtgEnyuUAnc73783926;     jtmLkmCzsEtgEnyuUAnc73783926 = jtmLkmCzsEtgEnyuUAnc47557905;     jtmLkmCzsEtgEnyuUAnc47557905 = jtmLkmCzsEtgEnyuUAnc87743762;     jtmLkmCzsEtgEnyuUAnc87743762 = jtmLkmCzsEtgEnyuUAnc82859696;     jtmLkmCzsEtgEnyuUAnc82859696 = jtmLkmCzsEtgEnyuUAnc19770518;     jtmLkmCzsEtgEnyuUAnc19770518 = jtmLkmCzsEtgEnyuUAnc84838622;     jtmLkmCzsEtgEnyuUAnc84838622 = jtmLkmCzsEtgEnyuUAnc88392080;     jtmLkmCzsEtgEnyuUAnc88392080 = jtmLkmCzsEtgEnyuUAnc64800208;     jtmLkmCzsEtgEnyuUAnc64800208 = jtmLkmCzsEtgEnyuUAnc35640336;     jtmLkmCzsEtgEnyuUAnc35640336 = jtmLkmCzsEtgEnyuUAnc90012764;     jtmLkmCzsEtgEnyuUAnc90012764 = jtmLkmCzsEtgEnyuUAnc55882772;     jtmLkmCzsEtgEnyuUAnc55882772 = jtmLkmCzsEtgEnyuUAnc31179662;     jtmLkmCzsEtgEnyuUAnc31179662 = jtmLkmCzsEtgEnyuUAnc22809504;     jtmLkmCzsEtgEnyuUAnc22809504 = jtmLkmCzsEtgEnyuUAnc50411975;     jtmLkmCzsEtgEnyuUAnc50411975 = jtmLkmCzsEtgEnyuUAnc93408521;     jtmLkmCzsEtgEnyuUAnc93408521 = jtmLkmCzsEtgEnyuUAnc1868987;     jtmLkmCzsEtgEnyuUAnc1868987 = jtmLkmCzsEtgEnyuUAnc17123076;     jtmLkmCzsEtgEnyuUAnc17123076 = jtmLkmCzsEtgEnyuUAnc44898747;     jtmLkmCzsEtgEnyuUAnc44898747 = jtmLkmCzsEtgEnyuUAnc98171786;     jtmLkmCzsEtgEnyuUAnc98171786 = jtmLkmCzsEtgEnyuUAnc18940043;     jtmLkmCzsEtgEnyuUAnc18940043 = jtmLkmCzsEtgEnyuUAnc63396619;     jtmLkmCzsEtgEnyuUAnc63396619 = jtmLkmCzsEtgEnyuUAnc84900918;     jtmLkmCzsEtgEnyuUAnc84900918 = jtmLkmCzsEtgEnyuUAnc90439555;     jtmLkmCzsEtgEnyuUAnc90439555 = jtmLkmCzsEtgEnyuUAnc11993560;     jtmLkmCzsEtgEnyuUAnc11993560 = jtmLkmCzsEtgEnyuUAnc77060485;     jtmLkmCzsEtgEnyuUAnc77060485 = jtmLkmCzsEtgEnyuUAnc66200227;     jtmLkmCzsEtgEnyuUAnc66200227 = jtmLkmCzsEtgEnyuUAnc9218374;     jtmLkmCzsEtgEnyuUAnc9218374 = jtmLkmCzsEtgEnyuUAnc79942929;     jtmLkmCzsEtgEnyuUAnc79942929 = jtmLkmCzsEtgEnyuUAnc9773014;     jtmLkmCzsEtgEnyuUAnc9773014 = jtmLkmCzsEtgEnyuUAnc74600072;     jtmLkmCzsEtgEnyuUAnc74600072 = jtmLkmCzsEtgEnyuUAnc36471149;     jtmLkmCzsEtgEnyuUAnc36471149 = jtmLkmCzsEtgEnyuUAnc67541180;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uabUXwKgBOokpOTgUFuB3639915() {     double JVoxlMBTIyJxOBRwjjtm93126457 = -239662582;    double JVoxlMBTIyJxOBRwjjtm88746988 = -918982165;    double JVoxlMBTIyJxOBRwjjtm105138 = -225250812;    double JVoxlMBTIyJxOBRwjjtm51107315 = -590617329;    double JVoxlMBTIyJxOBRwjjtm12507800 = -906045914;    double JVoxlMBTIyJxOBRwjjtm11440147 = -515558337;    double JVoxlMBTIyJxOBRwjjtm79355973 = -394519743;    double JVoxlMBTIyJxOBRwjjtm53554547 = -832702471;    double JVoxlMBTIyJxOBRwjjtm35472824 = -527948021;    double JVoxlMBTIyJxOBRwjjtm67798703 = -954538869;    double JVoxlMBTIyJxOBRwjjtm57227102 = -586998567;    double JVoxlMBTIyJxOBRwjjtm64998932 = -280030364;    double JVoxlMBTIyJxOBRwjjtm6976170 = -518713943;    double JVoxlMBTIyJxOBRwjjtm83950165 = -775180095;    double JVoxlMBTIyJxOBRwjjtm56383027 = -209843688;    double JVoxlMBTIyJxOBRwjjtm58052579 = -359172216;    double JVoxlMBTIyJxOBRwjjtm73540410 = -11325589;    double JVoxlMBTIyJxOBRwjjtm73915954 = -511289510;    double JVoxlMBTIyJxOBRwjjtm68155811 = -17116766;    double JVoxlMBTIyJxOBRwjjtm67175366 = -715311642;    double JVoxlMBTIyJxOBRwjjtm54115329 = -543098299;    double JVoxlMBTIyJxOBRwjjtm38477778 = -754519061;    double JVoxlMBTIyJxOBRwjjtm54684349 = -278759271;    double JVoxlMBTIyJxOBRwjjtm56605929 = -473511284;    double JVoxlMBTIyJxOBRwjjtm86283551 = -689783061;    double JVoxlMBTIyJxOBRwjjtm79775063 = 63851294;    double JVoxlMBTIyJxOBRwjjtm56098434 = -832560899;    double JVoxlMBTIyJxOBRwjjtm25977026 = -65870914;    double JVoxlMBTIyJxOBRwjjtm61650582 = -647020652;    double JVoxlMBTIyJxOBRwjjtm1064573 = -77086827;    double JVoxlMBTIyJxOBRwjjtm30006813 = -685706365;    double JVoxlMBTIyJxOBRwjjtm51675507 = -668496703;    double JVoxlMBTIyJxOBRwjjtm92222554 = -776378230;    double JVoxlMBTIyJxOBRwjjtm95466708 = -467187204;    double JVoxlMBTIyJxOBRwjjtm95368117 = -172095933;    double JVoxlMBTIyJxOBRwjjtm31122064 = -411114051;    double JVoxlMBTIyJxOBRwjjtm52419017 = -706818131;    double JVoxlMBTIyJxOBRwjjtm61571218 = -710888661;    double JVoxlMBTIyJxOBRwjjtm10863933 = -571720957;    double JVoxlMBTIyJxOBRwjjtm95704246 = -988834138;    double JVoxlMBTIyJxOBRwjjtm68326558 = -374457819;    double JVoxlMBTIyJxOBRwjjtm8723573 = -463131694;    double JVoxlMBTIyJxOBRwjjtm21374683 = -916992754;    double JVoxlMBTIyJxOBRwjjtm86073493 = -540097790;    double JVoxlMBTIyJxOBRwjjtm56859998 = -965087623;    double JVoxlMBTIyJxOBRwjjtm41897962 = -652315978;    double JVoxlMBTIyJxOBRwjjtm18874719 = -283168171;    double JVoxlMBTIyJxOBRwjjtm59013792 = -160597894;    double JVoxlMBTIyJxOBRwjjtm62477570 = -930713155;    double JVoxlMBTIyJxOBRwjjtm14174843 = -96839916;    double JVoxlMBTIyJxOBRwjjtm56009343 = -724250698;    double JVoxlMBTIyJxOBRwjjtm18095682 = -203694811;    double JVoxlMBTIyJxOBRwjjtm87660431 = 73747514;    double JVoxlMBTIyJxOBRwjjtm41762624 = -753590726;    double JVoxlMBTIyJxOBRwjjtm49524586 = -892509260;    double JVoxlMBTIyJxOBRwjjtm54648680 = -485143521;    double JVoxlMBTIyJxOBRwjjtm34062640 = -540222894;    double JVoxlMBTIyJxOBRwjjtm43499209 = -751739529;    double JVoxlMBTIyJxOBRwjjtm64823763 = -900834269;    double JVoxlMBTIyJxOBRwjjtm32732736 = -869897208;    double JVoxlMBTIyJxOBRwjjtm55341713 = -682997439;    double JVoxlMBTIyJxOBRwjjtm53378947 = -228648829;    double JVoxlMBTIyJxOBRwjjtm91903965 = -85681820;    double JVoxlMBTIyJxOBRwjjtm34408251 = -350861194;    double JVoxlMBTIyJxOBRwjjtm37791891 = -168832504;    double JVoxlMBTIyJxOBRwjjtm5551595 = -918501864;    double JVoxlMBTIyJxOBRwjjtm72776378 = -503652135;    double JVoxlMBTIyJxOBRwjjtm11509461 = 48473261;    double JVoxlMBTIyJxOBRwjjtm88582048 = -503084162;    double JVoxlMBTIyJxOBRwjjtm25260964 = -798729638;    double JVoxlMBTIyJxOBRwjjtm5633562 = -652354086;    double JVoxlMBTIyJxOBRwjjtm11969193 = -300436928;    double JVoxlMBTIyJxOBRwjjtm63052022 = -939568553;    double JVoxlMBTIyJxOBRwjjtm72451565 = -28282629;    double JVoxlMBTIyJxOBRwjjtm98848808 = -240853823;    double JVoxlMBTIyJxOBRwjjtm45391756 = 20033395;    double JVoxlMBTIyJxOBRwjjtm17103095 = -837526307;    double JVoxlMBTIyJxOBRwjjtm68610855 = -738661481;    double JVoxlMBTIyJxOBRwjjtm99745930 = -508423661;    double JVoxlMBTIyJxOBRwjjtm44385590 = 62532916;    double JVoxlMBTIyJxOBRwjjtm60900344 = -652980536;    double JVoxlMBTIyJxOBRwjjtm97084641 = -571963006;    double JVoxlMBTIyJxOBRwjjtm63499455 = -135157760;    double JVoxlMBTIyJxOBRwjjtm47475739 = -450180736;    double JVoxlMBTIyJxOBRwjjtm45055230 = -352836130;    double JVoxlMBTIyJxOBRwjjtm11911131 = -382011554;    double JVoxlMBTIyJxOBRwjjtm64015076 = -642244217;    double JVoxlMBTIyJxOBRwjjtm50459930 = 77212495;    double JVoxlMBTIyJxOBRwjjtm45942122 = -574677945;    double JVoxlMBTIyJxOBRwjjtm40719437 = -686952413;    double JVoxlMBTIyJxOBRwjjtm97059423 = -870891157;    double JVoxlMBTIyJxOBRwjjtm8919808 = -955078603;    double JVoxlMBTIyJxOBRwjjtm96747455 = -810054393;    double JVoxlMBTIyJxOBRwjjtm78131196 = -701823749;    double JVoxlMBTIyJxOBRwjjtm40362534 = -205836700;    double JVoxlMBTIyJxOBRwjjtm14947611 = -45808991;    double JVoxlMBTIyJxOBRwjjtm16819608 = -277449875;    double JVoxlMBTIyJxOBRwjjtm86966431 = -466131560;    double JVoxlMBTIyJxOBRwjjtm48281603 = -271265286;    double JVoxlMBTIyJxOBRwjjtm51308404 = -239662582;     JVoxlMBTIyJxOBRwjjtm93126457 = JVoxlMBTIyJxOBRwjjtm88746988;     JVoxlMBTIyJxOBRwjjtm88746988 = JVoxlMBTIyJxOBRwjjtm105138;     JVoxlMBTIyJxOBRwjjtm105138 = JVoxlMBTIyJxOBRwjjtm51107315;     JVoxlMBTIyJxOBRwjjtm51107315 = JVoxlMBTIyJxOBRwjjtm12507800;     JVoxlMBTIyJxOBRwjjtm12507800 = JVoxlMBTIyJxOBRwjjtm11440147;     JVoxlMBTIyJxOBRwjjtm11440147 = JVoxlMBTIyJxOBRwjjtm79355973;     JVoxlMBTIyJxOBRwjjtm79355973 = JVoxlMBTIyJxOBRwjjtm53554547;     JVoxlMBTIyJxOBRwjjtm53554547 = JVoxlMBTIyJxOBRwjjtm35472824;     JVoxlMBTIyJxOBRwjjtm35472824 = JVoxlMBTIyJxOBRwjjtm67798703;     JVoxlMBTIyJxOBRwjjtm67798703 = JVoxlMBTIyJxOBRwjjtm57227102;     JVoxlMBTIyJxOBRwjjtm57227102 = JVoxlMBTIyJxOBRwjjtm64998932;     JVoxlMBTIyJxOBRwjjtm64998932 = JVoxlMBTIyJxOBRwjjtm6976170;     JVoxlMBTIyJxOBRwjjtm6976170 = JVoxlMBTIyJxOBRwjjtm83950165;     JVoxlMBTIyJxOBRwjjtm83950165 = JVoxlMBTIyJxOBRwjjtm56383027;     JVoxlMBTIyJxOBRwjjtm56383027 = JVoxlMBTIyJxOBRwjjtm58052579;     JVoxlMBTIyJxOBRwjjtm58052579 = JVoxlMBTIyJxOBRwjjtm73540410;     JVoxlMBTIyJxOBRwjjtm73540410 = JVoxlMBTIyJxOBRwjjtm73915954;     JVoxlMBTIyJxOBRwjjtm73915954 = JVoxlMBTIyJxOBRwjjtm68155811;     JVoxlMBTIyJxOBRwjjtm68155811 = JVoxlMBTIyJxOBRwjjtm67175366;     JVoxlMBTIyJxOBRwjjtm67175366 = JVoxlMBTIyJxOBRwjjtm54115329;     JVoxlMBTIyJxOBRwjjtm54115329 = JVoxlMBTIyJxOBRwjjtm38477778;     JVoxlMBTIyJxOBRwjjtm38477778 = JVoxlMBTIyJxOBRwjjtm54684349;     JVoxlMBTIyJxOBRwjjtm54684349 = JVoxlMBTIyJxOBRwjjtm56605929;     JVoxlMBTIyJxOBRwjjtm56605929 = JVoxlMBTIyJxOBRwjjtm86283551;     JVoxlMBTIyJxOBRwjjtm86283551 = JVoxlMBTIyJxOBRwjjtm79775063;     JVoxlMBTIyJxOBRwjjtm79775063 = JVoxlMBTIyJxOBRwjjtm56098434;     JVoxlMBTIyJxOBRwjjtm56098434 = JVoxlMBTIyJxOBRwjjtm25977026;     JVoxlMBTIyJxOBRwjjtm25977026 = JVoxlMBTIyJxOBRwjjtm61650582;     JVoxlMBTIyJxOBRwjjtm61650582 = JVoxlMBTIyJxOBRwjjtm1064573;     JVoxlMBTIyJxOBRwjjtm1064573 = JVoxlMBTIyJxOBRwjjtm30006813;     JVoxlMBTIyJxOBRwjjtm30006813 = JVoxlMBTIyJxOBRwjjtm51675507;     JVoxlMBTIyJxOBRwjjtm51675507 = JVoxlMBTIyJxOBRwjjtm92222554;     JVoxlMBTIyJxOBRwjjtm92222554 = JVoxlMBTIyJxOBRwjjtm95466708;     JVoxlMBTIyJxOBRwjjtm95466708 = JVoxlMBTIyJxOBRwjjtm95368117;     JVoxlMBTIyJxOBRwjjtm95368117 = JVoxlMBTIyJxOBRwjjtm31122064;     JVoxlMBTIyJxOBRwjjtm31122064 = JVoxlMBTIyJxOBRwjjtm52419017;     JVoxlMBTIyJxOBRwjjtm52419017 = JVoxlMBTIyJxOBRwjjtm61571218;     JVoxlMBTIyJxOBRwjjtm61571218 = JVoxlMBTIyJxOBRwjjtm10863933;     JVoxlMBTIyJxOBRwjjtm10863933 = JVoxlMBTIyJxOBRwjjtm95704246;     JVoxlMBTIyJxOBRwjjtm95704246 = JVoxlMBTIyJxOBRwjjtm68326558;     JVoxlMBTIyJxOBRwjjtm68326558 = JVoxlMBTIyJxOBRwjjtm8723573;     JVoxlMBTIyJxOBRwjjtm8723573 = JVoxlMBTIyJxOBRwjjtm21374683;     JVoxlMBTIyJxOBRwjjtm21374683 = JVoxlMBTIyJxOBRwjjtm86073493;     JVoxlMBTIyJxOBRwjjtm86073493 = JVoxlMBTIyJxOBRwjjtm56859998;     JVoxlMBTIyJxOBRwjjtm56859998 = JVoxlMBTIyJxOBRwjjtm41897962;     JVoxlMBTIyJxOBRwjjtm41897962 = JVoxlMBTIyJxOBRwjjtm18874719;     JVoxlMBTIyJxOBRwjjtm18874719 = JVoxlMBTIyJxOBRwjjtm59013792;     JVoxlMBTIyJxOBRwjjtm59013792 = JVoxlMBTIyJxOBRwjjtm62477570;     JVoxlMBTIyJxOBRwjjtm62477570 = JVoxlMBTIyJxOBRwjjtm14174843;     JVoxlMBTIyJxOBRwjjtm14174843 = JVoxlMBTIyJxOBRwjjtm56009343;     JVoxlMBTIyJxOBRwjjtm56009343 = JVoxlMBTIyJxOBRwjjtm18095682;     JVoxlMBTIyJxOBRwjjtm18095682 = JVoxlMBTIyJxOBRwjjtm87660431;     JVoxlMBTIyJxOBRwjjtm87660431 = JVoxlMBTIyJxOBRwjjtm41762624;     JVoxlMBTIyJxOBRwjjtm41762624 = JVoxlMBTIyJxOBRwjjtm49524586;     JVoxlMBTIyJxOBRwjjtm49524586 = JVoxlMBTIyJxOBRwjjtm54648680;     JVoxlMBTIyJxOBRwjjtm54648680 = JVoxlMBTIyJxOBRwjjtm34062640;     JVoxlMBTIyJxOBRwjjtm34062640 = JVoxlMBTIyJxOBRwjjtm43499209;     JVoxlMBTIyJxOBRwjjtm43499209 = JVoxlMBTIyJxOBRwjjtm64823763;     JVoxlMBTIyJxOBRwjjtm64823763 = JVoxlMBTIyJxOBRwjjtm32732736;     JVoxlMBTIyJxOBRwjjtm32732736 = JVoxlMBTIyJxOBRwjjtm55341713;     JVoxlMBTIyJxOBRwjjtm55341713 = JVoxlMBTIyJxOBRwjjtm53378947;     JVoxlMBTIyJxOBRwjjtm53378947 = JVoxlMBTIyJxOBRwjjtm91903965;     JVoxlMBTIyJxOBRwjjtm91903965 = JVoxlMBTIyJxOBRwjjtm34408251;     JVoxlMBTIyJxOBRwjjtm34408251 = JVoxlMBTIyJxOBRwjjtm37791891;     JVoxlMBTIyJxOBRwjjtm37791891 = JVoxlMBTIyJxOBRwjjtm5551595;     JVoxlMBTIyJxOBRwjjtm5551595 = JVoxlMBTIyJxOBRwjjtm72776378;     JVoxlMBTIyJxOBRwjjtm72776378 = JVoxlMBTIyJxOBRwjjtm11509461;     JVoxlMBTIyJxOBRwjjtm11509461 = JVoxlMBTIyJxOBRwjjtm88582048;     JVoxlMBTIyJxOBRwjjtm88582048 = JVoxlMBTIyJxOBRwjjtm25260964;     JVoxlMBTIyJxOBRwjjtm25260964 = JVoxlMBTIyJxOBRwjjtm5633562;     JVoxlMBTIyJxOBRwjjtm5633562 = JVoxlMBTIyJxOBRwjjtm11969193;     JVoxlMBTIyJxOBRwjjtm11969193 = JVoxlMBTIyJxOBRwjjtm63052022;     JVoxlMBTIyJxOBRwjjtm63052022 = JVoxlMBTIyJxOBRwjjtm72451565;     JVoxlMBTIyJxOBRwjjtm72451565 = JVoxlMBTIyJxOBRwjjtm98848808;     JVoxlMBTIyJxOBRwjjtm98848808 = JVoxlMBTIyJxOBRwjjtm45391756;     JVoxlMBTIyJxOBRwjjtm45391756 = JVoxlMBTIyJxOBRwjjtm17103095;     JVoxlMBTIyJxOBRwjjtm17103095 = JVoxlMBTIyJxOBRwjjtm68610855;     JVoxlMBTIyJxOBRwjjtm68610855 = JVoxlMBTIyJxOBRwjjtm99745930;     JVoxlMBTIyJxOBRwjjtm99745930 = JVoxlMBTIyJxOBRwjjtm44385590;     JVoxlMBTIyJxOBRwjjtm44385590 = JVoxlMBTIyJxOBRwjjtm60900344;     JVoxlMBTIyJxOBRwjjtm60900344 = JVoxlMBTIyJxOBRwjjtm97084641;     JVoxlMBTIyJxOBRwjjtm97084641 = JVoxlMBTIyJxOBRwjjtm63499455;     JVoxlMBTIyJxOBRwjjtm63499455 = JVoxlMBTIyJxOBRwjjtm47475739;     JVoxlMBTIyJxOBRwjjtm47475739 = JVoxlMBTIyJxOBRwjjtm45055230;     JVoxlMBTIyJxOBRwjjtm45055230 = JVoxlMBTIyJxOBRwjjtm11911131;     JVoxlMBTIyJxOBRwjjtm11911131 = JVoxlMBTIyJxOBRwjjtm64015076;     JVoxlMBTIyJxOBRwjjtm64015076 = JVoxlMBTIyJxOBRwjjtm50459930;     JVoxlMBTIyJxOBRwjjtm50459930 = JVoxlMBTIyJxOBRwjjtm45942122;     JVoxlMBTIyJxOBRwjjtm45942122 = JVoxlMBTIyJxOBRwjjtm40719437;     JVoxlMBTIyJxOBRwjjtm40719437 = JVoxlMBTIyJxOBRwjjtm97059423;     JVoxlMBTIyJxOBRwjjtm97059423 = JVoxlMBTIyJxOBRwjjtm8919808;     JVoxlMBTIyJxOBRwjjtm8919808 = JVoxlMBTIyJxOBRwjjtm96747455;     JVoxlMBTIyJxOBRwjjtm96747455 = JVoxlMBTIyJxOBRwjjtm78131196;     JVoxlMBTIyJxOBRwjjtm78131196 = JVoxlMBTIyJxOBRwjjtm40362534;     JVoxlMBTIyJxOBRwjjtm40362534 = JVoxlMBTIyJxOBRwjjtm14947611;     JVoxlMBTIyJxOBRwjjtm14947611 = JVoxlMBTIyJxOBRwjjtm16819608;     JVoxlMBTIyJxOBRwjjtm16819608 = JVoxlMBTIyJxOBRwjjtm86966431;     JVoxlMBTIyJxOBRwjjtm86966431 = JVoxlMBTIyJxOBRwjjtm48281603;     JVoxlMBTIyJxOBRwjjtm48281603 = JVoxlMBTIyJxOBRwjjtm51308404;     JVoxlMBTIyJxOBRwjjtm51308404 = JVoxlMBTIyJxOBRwjjtm93126457;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MsXxfZGOmxfCdLXEUwOc31234172() {     double NUKuAeSIbHpVcEHChjCW18711735 = -764695682;    double NUKuAeSIbHpVcEHChjCW85144131 = -74459941;    double NUKuAeSIbHpVcEHChjCW32984313 = -27712348;    double NUKuAeSIbHpVcEHChjCW96829244 = -597940223;    double NUKuAeSIbHpVcEHChjCW81849804 = -952968201;    double NUKuAeSIbHpVcEHChjCW28449424 = -670539341;    double NUKuAeSIbHpVcEHChjCW84676817 = -976790950;    double NUKuAeSIbHpVcEHChjCW68564671 = -113384350;    double NUKuAeSIbHpVcEHChjCW60802116 = 75827934;    double NUKuAeSIbHpVcEHChjCW86957086 = -189100392;    double NUKuAeSIbHpVcEHChjCW71560655 = -897426453;    double NUKuAeSIbHpVcEHChjCW316589 = -723739271;    double NUKuAeSIbHpVcEHChjCW83556947 = -282962308;    double NUKuAeSIbHpVcEHChjCW75509882 = -749704389;    double NUKuAeSIbHpVcEHChjCW12651915 = -98389646;    double NUKuAeSIbHpVcEHChjCW65773621 = -795363693;    double NUKuAeSIbHpVcEHChjCW73338527 = -799645383;    double NUKuAeSIbHpVcEHChjCW95031859 = 72433129;    double NUKuAeSIbHpVcEHChjCW29620533 = -800090624;    double NUKuAeSIbHpVcEHChjCW28336833 = -689751017;    double NUKuAeSIbHpVcEHChjCW55124594 = 95201920;    double NUKuAeSIbHpVcEHChjCW39980884 = -74251990;    double NUKuAeSIbHpVcEHChjCW58448146 = -322851376;    double NUKuAeSIbHpVcEHChjCW58319971 = -332360263;    double NUKuAeSIbHpVcEHChjCW41186691 = -257279203;    double NUKuAeSIbHpVcEHChjCW9264200 = -600599248;    double NUKuAeSIbHpVcEHChjCW38486343 = -398374034;    double NUKuAeSIbHpVcEHChjCW89875825 = -295533147;    double NUKuAeSIbHpVcEHChjCW69527794 = -883314502;    double NUKuAeSIbHpVcEHChjCW54387064 = 14003611;    double NUKuAeSIbHpVcEHChjCW52053447 = -464353396;    double NUKuAeSIbHpVcEHChjCW88865440 = -942292537;    double NUKuAeSIbHpVcEHChjCW13409606 = -124199367;    double NUKuAeSIbHpVcEHChjCW49053779 = -523332176;    double NUKuAeSIbHpVcEHChjCW96773105 = -873531845;    double NUKuAeSIbHpVcEHChjCW35913914 = -928708766;    double NUKuAeSIbHpVcEHChjCW2064403 = -87330076;    double NUKuAeSIbHpVcEHChjCW37143905 = -559603486;    double NUKuAeSIbHpVcEHChjCW51787511 = 69940721;    double NUKuAeSIbHpVcEHChjCW4487921 = -358804457;    double NUKuAeSIbHpVcEHChjCW15477839 = -949518009;    double NUKuAeSIbHpVcEHChjCW52733163 = -972170198;    double NUKuAeSIbHpVcEHChjCW70574902 = -945599136;    double NUKuAeSIbHpVcEHChjCW56866773 = 91486611;    double NUKuAeSIbHpVcEHChjCW48840874 = -904108741;    double NUKuAeSIbHpVcEHChjCW8298285 = -74863515;    double NUKuAeSIbHpVcEHChjCW18643173 = -315637031;    double NUKuAeSIbHpVcEHChjCW67126563 = -353780741;    double NUKuAeSIbHpVcEHChjCW13288890 = 69610618;    double NUKuAeSIbHpVcEHChjCW67984837 = 55136931;    double NUKuAeSIbHpVcEHChjCW66145591 = -972909300;    double NUKuAeSIbHpVcEHChjCW45354262 = -381773991;    double NUKuAeSIbHpVcEHChjCW5734036 = -494151035;    double NUKuAeSIbHpVcEHChjCW10661534 = 31345492;    double NUKuAeSIbHpVcEHChjCW76109577 = -267599677;    double NUKuAeSIbHpVcEHChjCW78730850 = -590443692;    double NUKuAeSIbHpVcEHChjCW26695986 = -751608566;    double NUKuAeSIbHpVcEHChjCW74664342 = -695352085;    double NUKuAeSIbHpVcEHChjCW55642553 = -240661020;    double NUKuAeSIbHpVcEHChjCW72585604 = -252368954;    double NUKuAeSIbHpVcEHChjCW89963080 = -172165307;    double NUKuAeSIbHpVcEHChjCW94800991 = -581257803;    double NUKuAeSIbHpVcEHChjCW99036877 = -230069848;    double NUKuAeSIbHpVcEHChjCW6415053 = -938175677;    double NUKuAeSIbHpVcEHChjCW34903639 = -724746997;    double NUKuAeSIbHpVcEHChjCW82695215 = -955133917;    double NUKuAeSIbHpVcEHChjCW86906982 = -499539904;    double NUKuAeSIbHpVcEHChjCW34503168 = -759630133;    double NUKuAeSIbHpVcEHChjCW78736776 = -876172544;    double NUKuAeSIbHpVcEHChjCW76738001 = -169680880;    double NUKuAeSIbHpVcEHChjCW63709219 = -608033618;    double NUKuAeSIbHpVcEHChjCW36194622 = -140041897;    double NUKuAeSIbHpVcEHChjCW43244348 = -997507592;    double NUKuAeSIbHpVcEHChjCW25132613 = -341286167;    double NUKuAeSIbHpVcEHChjCW12858994 = -740233009;    double NUKuAeSIbHpVcEHChjCW2391432 = 67372118;    double NUKuAeSIbHpVcEHChjCW69405982 = -128652855;    double NUKuAeSIbHpVcEHChjCW1581374 = -314337987;    double NUKuAeSIbHpVcEHChjCW9479098 = -428251523;    double NUKuAeSIbHpVcEHChjCW32888407 = -82415689;    double NUKuAeSIbHpVcEHChjCW90621026 = -184962217;    double NUKuAeSIbHpVcEHChjCW71359780 = 55406706;    double NUKuAeSIbHpVcEHChjCW76586935 = -265143766;    double NUKuAeSIbHpVcEHChjCW1542957 = -838451433;    double NUKuAeSIbHpVcEHChjCW88241473 = -13087089;    double NUKuAeSIbHpVcEHChjCW6699186 = 17420595;    double NUKuAeSIbHpVcEHChjCW83131404 = -348141502;    double NUKuAeSIbHpVcEHChjCW2748073 = -55544860;    double NUKuAeSIbHpVcEHChjCW72944202 = -155732499;    double NUKuAeSIbHpVcEHChjCW18042256 = -183088153;    double NUKuAeSIbHpVcEHChjCW9217929 = -77100201;    double NUKuAeSIbHpVcEHChjCW27400060 = -391977991;    double NUKuAeSIbHpVcEHChjCW81501352 = -218942467;    double NUKuAeSIbHpVcEHChjCW79201906 = -677690326;    double NUKuAeSIbHpVcEHChjCW14524840 = -86639150;    double NUKuAeSIbHpVcEHChjCW20676847 = -268260206;    double NUKuAeSIbHpVcEHChjCW53696285 = -642100351;    double NUKuAeSIbHpVcEHChjCW64159849 = 92576541;    double NUKuAeSIbHpVcEHChjCW21963134 = -183766393;    double NUKuAeSIbHpVcEHChjCW66145658 = -764695682;     NUKuAeSIbHpVcEHChjCW18711735 = NUKuAeSIbHpVcEHChjCW85144131;     NUKuAeSIbHpVcEHChjCW85144131 = NUKuAeSIbHpVcEHChjCW32984313;     NUKuAeSIbHpVcEHChjCW32984313 = NUKuAeSIbHpVcEHChjCW96829244;     NUKuAeSIbHpVcEHChjCW96829244 = NUKuAeSIbHpVcEHChjCW81849804;     NUKuAeSIbHpVcEHChjCW81849804 = NUKuAeSIbHpVcEHChjCW28449424;     NUKuAeSIbHpVcEHChjCW28449424 = NUKuAeSIbHpVcEHChjCW84676817;     NUKuAeSIbHpVcEHChjCW84676817 = NUKuAeSIbHpVcEHChjCW68564671;     NUKuAeSIbHpVcEHChjCW68564671 = NUKuAeSIbHpVcEHChjCW60802116;     NUKuAeSIbHpVcEHChjCW60802116 = NUKuAeSIbHpVcEHChjCW86957086;     NUKuAeSIbHpVcEHChjCW86957086 = NUKuAeSIbHpVcEHChjCW71560655;     NUKuAeSIbHpVcEHChjCW71560655 = NUKuAeSIbHpVcEHChjCW316589;     NUKuAeSIbHpVcEHChjCW316589 = NUKuAeSIbHpVcEHChjCW83556947;     NUKuAeSIbHpVcEHChjCW83556947 = NUKuAeSIbHpVcEHChjCW75509882;     NUKuAeSIbHpVcEHChjCW75509882 = NUKuAeSIbHpVcEHChjCW12651915;     NUKuAeSIbHpVcEHChjCW12651915 = NUKuAeSIbHpVcEHChjCW65773621;     NUKuAeSIbHpVcEHChjCW65773621 = NUKuAeSIbHpVcEHChjCW73338527;     NUKuAeSIbHpVcEHChjCW73338527 = NUKuAeSIbHpVcEHChjCW95031859;     NUKuAeSIbHpVcEHChjCW95031859 = NUKuAeSIbHpVcEHChjCW29620533;     NUKuAeSIbHpVcEHChjCW29620533 = NUKuAeSIbHpVcEHChjCW28336833;     NUKuAeSIbHpVcEHChjCW28336833 = NUKuAeSIbHpVcEHChjCW55124594;     NUKuAeSIbHpVcEHChjCW55124594 = NUKuAeSIbHpVcEHChjCW39980884;     NUKuAeSIbHpVcEHChjCW39980884 = NUKuAeSIbHpVcEHChjCW58448146;     NUKuAeSIbHpVcEHChjCW58448146 = NUKuAeSIbHpVcEHChjCW58319971;     NUKuAeSIbHpVcEHChjCW58319971 = NUKuAeSIbHpVcEHChjCW41186691;     NUKuAeSIbHpVcEHChjCW41186691 = NUKuAeSIbHpVcEHChjCW9264200;     NUKuAeSIbHpVcEHChjCW9264200 = NUKuAeSIbHpVcEHChjCW38486343;     NUKuAeSIbHpVcEHChjCW38486343 = NUKuAeSIbHpVcEHChjCW89875825;     NUKuAeSIbHpVcEHChjCW89875825 = NUKuAeSIbHpVcEHChjCW69527794;     NUKuAeSIbHpVcEHChjCW69527794 = NUKuAeSIbHpVcEHChjCW54387064;     NUKuAeSIbHpVcEHChjCW54387064 = NUKuAeSIbHpVcEHChjCW52053447;     NUKuAeSIbHpVcEHChjCW52053447 = NUKuAeSIbHpVcEHChjCW88865440;     NUKuAeSIbHpVcEHChjCW88865440 = NUKuAeSIbHpVcEHChjCW13409606;     NUKuAeSIbHpVcEHChjCW13409606 = NUKuAeSIbHpVcEHChjCW49053779;     NUKuAeSIbHpVcEHChjCW49053779 = NUKuAeSIbHpVcEHChjCW96773105;     NUKuAeSIbHpVcEHChjCW96773105 = NUKuAeSIbHpVcEHChjCW35913914;     NUKuAeSIbHpVcEHChjCW35913914 = NUKuAeSIbHpVcEHChjCW2064403;     NUKuAeSIbHpVcEHChjCW2064403 = NUKuAeSIbHpVcEHChjCW37143905;     NUKuAeSIbHpVcEHChjCW37143905 = NUKuAeSIbHpVcEHChjCW51787511;     NUKuAeSIbHpVcEHChjCW51787511 = NUKuAeSIbHpVcEHChjCW4487921;     NUKuAeSIbHpVcEHChjCW4487921 = NUKuAeSIbHpVcEHChjCW15477839;     NUKuAeSIbHpVcEHChjCW15477839 = NUKuAeSIbHpVcEHChjCW52733163;     NUKuAeSIbHpVcEHChjCW52733163 = NUKuAeSIbHpVcEHChjCW70574902;     NUKuAeSIbHpVcEHChjCW70574902 = NUKuAeSIbHpVcEHChjCW56866773;     NUKuAeSIbHpVcEHChjCW56866773 = NUKuAeSIbHpVcEHChjCW48840874;     NUKuAeSIbHpVcEHChjCW48840874 = NUKuAeSIbHpVcEHChjCW8298285;     NUKuAeSIbHpVcEHChjCW8298285 = NUKuAeSIbHpVcEHChjCW18643173;     NUKuAeSIbHpVcEHChjCW18643173 = NUKuAeSIbHpVcEHChjCW67126563;     NUKuAeSIbHpVcEHChjCW67126563 = NUKuAeSIbHpVcEHChjCW13288890;     NUKuAeSIbHpVcEHChjCW13288890 = NUKuAeSIbHpVcEHChjCW67984837;     NUKuAeSIbHpVcEHChjCW67984837 = NUKuAeSIbHpVcEHChjCW66145591;     NUKuAeSIbHpVcEHChjCW66145591 = NUKuAeSIbHpVcEHChjCW45354262;     NUKuAeSIbHpVcEHChjCW45354262 = NUKuAeSIbHpVcEHChjCW5734036;     NUKuAeSIbHpVcEHChjCW5734036 = NUKuAeSIbHpVcEHChjCW10661534;     NUKuAeSIbHpVcEHChjCW10661534 = NUKuAeSIbHpVcEHChjCW76109577;     NUKuAeSIbHpVcEHChjCW76109577 = NUKuAeSIbHpVcEHChjCW78730850;     NUKuAeSIbHpVcEHChjCW78730850 = NUKuAeSIbHpVcEHChjCW26695986;     NUKuAeSIbHpVcEHChjCW26695986 = NUKuAeSIbHpVcEHChjCW74664342;     NUKuAeSIbHpVcEHChjCW74664342 = NUKuAeSIbHpVcEHChjCW55642553;     NUKuAeSIbHpVcEHChjCW55642553 = NUKuAeSIbHpVcEHChjCW72585604;     NUKuAeSIbHpVcEHChjCW72585604 = NUKuAeSIbHpVcEHChjCW89963080;     NUKuAeSIbHpVcEHChjCW89963080 = NUKuAeSIbHpVcEHChjCW94800991;     NUKuAeSIbHpVcEHChjCW94800991 = NUKuAeSIbHpVcEHChjCW99036877;     NUKuAeSIbHpVcEHChjCW99036877 = NUKuAeSIbHpVcEHChjCW6415053;     NUKuAeSIbHpVcEHChjCW6415053 = NUKuAeSIbHpVcEHChjCW34903639;     NUKuAeSIbHpVcEHChjCW34903639 = NUKuAeSIbHpVcEHChjCW82695215;     NUKuAeSIbHpVcEHChjCW82695215 = NUKuAeSIbHpVcEHChjCW86906982;     NUKuAeSIbHpVcEHChjCW86906982 = NUKuAeSIbHpVcEHChjCW34503168;     NUKuAeSIbHpVcEHChjCW34503168 = NUKuAeSIbHpVcEHChjCW78736776;     NUKuAeSIbHpVcEHChjCW78736776 = NUKuAeSIbHpVcEHChjCW76738001;     NUKuAeSIbHpVcEHChjCW76738001 = NUKuAeSIbHpVcEHChjCW63709219;     NUKuAeSIbHpVcEHChjCW63709219 = NUKuAeSIbHpVcEHChjCW36194622;     NUKuAeSIbHpVcEHChjCW36194622 = NUKuAeSIbHpVcEHChjCW43244348;     NUKuAeSIbHpVcEHChjCW43244348 = NUKuAeSIbHpVcEHChjCW25132613;     NUKuAeSIbHpVcEHChjCW25132613 = NUKuAeSIbHpVcEHChjCW12858994;     NUKuAeSIbHpVcEHChjCW12858994 = NUKuAeSIbHpVcEHChjCW2391432;     NUKuAeSIbHpVcEHChjCW2391432 = NUKuAeSIbHpVcEHChjCW69405982;     NUKuAeSIbHpVcEHChjCW69405982 = NUKuAeSIbHpVcEHChjCW1581374;     NUKuAeSIbHpVcEHChjCW1581374 = NUKuAeSIbHpVcEHChjCW9479098;     NUKuAeSIbHpVcEHChjCW9479098 = NUKuAeSIbHpVcEHChjCW32888407;     NUKuAeSIbHpVcEHChjCW32888407 = NUKuAeSIbHpVcEHChjCW90621026;     NUKuAeSIbHpVcEHChjCW90621026 = NUKuAeSIbHpVcEHChjCW71359780;     NUKuAeSIbHpVcEHChjCW71359780 = NUKuAeSIbHpVcEHChjCW76586935;     NUKuAeSIbHpVcEHChjCW76586935 = NUKuAeSIbHpVcEHChjCW1542957;     NUKuAeSIbHpVcEHChjCW1542957 = NUKuAeSIbHpVcEHChjCW88241473;     NUKuAeSIbHpVcEHChjCW88241473 = NUKuAeSIbHpVcEHChjCW6699186;     NUKuAeSIbHpVcEHChjCW6699186 = NUKuAeSIbHpVcEHChjCW83131404;     NUKuAeSIbHpVcEHChjCW83131404 = NUKuAeSIbHpVcEHChjCW2748073;     NUKuAeSIbHpVcEHChjCW2748073 = NUKuAeSIbHpVcEHChjCW72944202;     NUKuAeSIbHpVcEHChjCW72944202 = NUKuAeSIbHpVcEHChjCW18042256;     NUKuAeSIbHpVcEHChjCW18042256 = NUKuAeSIbHpVcEHChjCW9217929;     NUKuAeSIbHpVcEHChjCW9217929 = NUKuAeSIbHpVcEHChjCW27400060;     NUKuAeSIbHpVcEHChjCW27400060 = NUKuAeSIbHpVcEHChjCW81501352;     NUKuAeSIbHpVcEHChjCW81501352 = NUKuAeSIbHpVcEHChjCW79201906;     NUKuAeSIbHpVcEHChjCW79201906 = NUKuAeSIbHpVcEHChjCW14524840;     NUKuAeSIbHpVcEHChjCW14524840 = NUKuAeSIbHpVcEHChjCW20676847;     NUKuAeSIbHpVcEHChjCW20676847 = NUKuAeSIbHpVcEHChjCW53696285;     NUKuAeSIbHpVcEHChjCW53696285 = NUKuAeSIbHpVcEHChjCW64159849;     NUKuAeSIbHpVcEHChjCW64159849 = NUKuAeSIbHpVcEHChjCW21963134;     NUKuAeSIbHpVcEHChjCW21963134 = NUKuAeSIbHpVcEHChjCW66145658;     NUKuAeSIbHpVcEHChjCW66145658 = NUKuAeSIbHpVcEHChjCW18711735;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void twcXuVoDRZAmaLcXwlvc58828428() {     double GfQCdahuXMIZaERbuydx44297012 = -189728783;    double GfQCdahuXMIZaERbuydx81541275 = -329937718;    double GfQCdahuXMIZaERbuydx65863489 = -930173884;    double GfQCdahuXMIZaERbuydx42551174 = -605263116;    double GfQCdahuXMIZaERbuydx51191809 = -999890488;    double GfQCdahuXMIZaERbuydx45458700 = -825520344;    double GfQCdahuXMIZaERbuydx89997662 = -459062157;    double GfQCdahuXMIZaERbuydx83574795 = -494066229;    double GfQCdahuXMIZaERbuydx86131409 = -420396110;    double GfQCdahuXMIZaERbuydx6115469 = -523661915;    double GfQCdahuXMIZaERbuydx85894209 = -107854340;    double GfQCdahuXMIZaERbuydx35634245 = -67448178;    double GfQCdahuXMIZaERbuydx60137725 = -47210673;    double GfQCdahuXMIZaERbuydx67069599 = -724228682;    double GfQCdahuXMIZaERbuydx68920803 = 13064397;    double GfQCdahuXMIZaERbuydx73494664 = -131555171;    double GfQCdahuXMIZaERbuydx73136643 = -487965177;    double GfQCdahuXMIZaERbuydx16147764 = -443844231;    double GfQCdahuXMIZaERbuydx91085253 = -483064481;    double GfQCdahuXMIZaERbuydx89498299 = -664190393;    double GfQCdahuXMIZaERbuydx56133859 = -366497861;    double GfQCdahuXMIZaERbuydx41483991 = -493984920;    double GfQCdahuXMIZaERbuydx62211944 = -366943481;    double GfQCdahuXMIZaERbuydx60034014 = -191209243;    double GfQCdahuXMIZaERbuydx96089830 = -924775346;    double GfQCdahuXMIZaERbuydx38753336 = -165049789;    double GfQCdahuXMIZaERbuydx20874252 = 35812832;    double GfQCdahuXMIZaERbuydx53774626 = -525195379;    double GfQCdahuXMIZaERbuydx77405006 = -19608353;    double GfQCdahuXMIZaERbuydx7709555 = -994905951;    double GfQCdahuXMIZaERbuydx74100082 = -243000427;    double GfQCdahuXMIZaERbuydx26055373 = -116088371;    double GfQCdahuXMIZaERbuydx34596658 = -572020504;    double GfQCdahuXMIZaERbuydx2640851 = -579477147;    double GfQCdahuXMIZaERbuydx98178093 = -474967756;    double GfQCdahuXMIZaERbuydx40705765 = -346303482;    double GfQCdahuXMIZaERbuydx51709787 = -567842020;    double GfQCdahuXMIZaERbuydx12716592 = -408318311;    double GfQCdahuXMIZaERbuydx92711089 = -388397601;    double GfQCdahuXMIZaERbuydx13271594 = -828774776;    double GfQCdahuXMIZaERbuydx62629120 = -424578199;    double GfQCdahuXMIZaERbuydx96742752 = -381208702;    double GfQCdahuXMIZaERbuydx19775122 = -974205517;    double GfQCdahuXMIZaERbuydx27660052 = -376928988;    double GfQCdahuXMIZaERbuydx40821750 = -843129858;    double GfQCdahuXMIZaERbuydx74698606 = -597411053;    double GfQCdahuXMIZaERbuydx18411628 = -348105891;    double GfQCdahuXMIZaERbuydx75239334 = -546963587;    double GfQCdahuXMIZaERbuydx64100210 = -30065608;    double GfQCdahuXMIZaERbuydx21794833 = -892886223;    double GfQCdahuXMIZaERbuydx76281838 = -121567902;    double GfQCdahuXMIZaERbuydx72612841 = -559853170;    double GfQCdahuXMIZaERbuydx23807639 = 37950416;    double GfQCdahuXMIZaERbuydx79560442 = -283718289;    double GfQCdahuXMIZaERbuydx2694569 = -742690095;    double GfQCdahuXMIZaERbuydx2813021 = -695743864;    double GfQCdahuXMIZaERbuydx19329331 = -962994237;    double GfQCdahuXMIZaERbuydx5829475 = -638964642;    double GfQCdahuXMIZaERbuydx46461344 = -680487771;    double GfQCdahuXMIZaERbuydx12438473 = -734840699;    double GfQCdahuXMIZaERbuydx24584448 = -761333176;    double GfQCdahuXMIZaERbuydx36223036 = -933866778;    double GfQCdahuXMIZaERbuydx6169790 = -374457877;    double GfQCdahuXMIZaERbuydx78421854 = -425490160;    double GfQCdahuXMIZaERbuydx32015387 = -180661489;    double GfQCdahuXMIZaERbuydx59838837 = -991765970;    double GfQCdahuXMIZaERbuydx1037587 = -495427674;    double GfQCdahuXMIZaERbuydx57496874 = -467733526;    double GfQCdahuXMIZaERbuydx68891505 = -149260926;    double GfQCdahuXMIZaERbuydx28215039 = -640632121;    double GfQCdahuXMIZaERbuydx21784877 = -563713151;    double GfQCdahuXMIZaERbuydx60420051 = 20353134;    double GfQCdahuXMIZaERbuydx23436674 = 44553369;    double GfQCdahuXMIZaERbuydx77813659 = -654289706;    double GfQCdahuXMIZaERbuydx26869179 = -139612195;    double GfQCdahuXMIZaERbuydx59391107 = -985289159;    double GfQCdahuXMIZaERbuydx21708870 = -519779403;    double GfQCdahuXMIZaERbuydx34551892 = -990014494;    double GfQCdahuXMIZaERbuydx19212264 = -348079386;    double GfQCdahuXMIZaERbuydx21391224 = -227364293;    double GfQCdahuXMIZaERbuydx20341709 = -816943899;    double GfQCdahuXMIZaERbuydx45634918 = -417223582;    double GfQCdahuXMIZaERbuydx89674415 = -395129772;    double GfQCdahuXMIZaERbuydx55610174 = -126722131;    double GfQCdahuXMIZaERbuydx31427717 = -773338049;    double GfQCdahuXMIZaERbuydx1487241 = -683147257;    double GfQCdahuXMIZaERbuydx2247734 = -54038787;    double GfQCdahuXMIZaERbuydx55036216 = -188302216;    double GfQCdahuXMIZaERbuydx99946281 = -836787053;    double GfQCdahuXMIZaERbuydx95365073 = -779223893;    double GfQCdahuXMIZaERbuydx21376434 = -383309246;    double GfQCdahuXMIZaERbuydx45880312 = -928877379;    double GfQCdahuXMIZaERbuydx66255248 = -727830541;    double GfQCdahuXMIZaERbuydx80272617 = -653556902;    double GfQCdahuXMIZaERbuydx88687146 = 32558399;    double GfQCdahuXMIZaERbuydx26406084 = -490711421;    double GfQCdahuXMIZaERbuydx90572963 = 93249174;    double GfQCdahuXMIZaERbuydx41353267 = -448715357;    double GfQCdahuXMIZaERbuydx95644665 = -96267499;    double GfQCdahuXMIZaERbuydx80982913 = -189728783;     GfQCdahuXMIZaERbuydx44297012 = GfQCdahuXMIZaERbuydx81541275;     GfQCdahuXMIZaERbuydx81541275 = GfQCdahuXMIZaERbuydx65863489;     GfQCdahuXMIZaERbuydx65863489 = GfQCdahuXMIZaERbuydx42551174;     GfQCdahuXMIZaERbuydx42551174 = GfQCdahuXMIZaERbuydx51191809;     GfQCdahuXMIZaERbuydx51191809 = GfQCdahuXMIZaERbuydx45458700;     GfQCdahuXMIZaERbuydx45458700 = GfQCdahuXMIZaERbuydx89997662;     GfQCdahuXMIZaERbuydx89997662 = GfQCdahuXMIZaERbuydx83574795;     GfQCdahuXMIZaERbuydx83574795 = GfQCdahuXMIZaERbuydx86131409;     GfQCdahuXMIZaERbuydx86131409 = GfQCdahuXMIZaERbuydx6115469;     GfQCdahuXMIZaERbuydx6115469 = GfQCdahuXMIZaERbuydx85894209;     GfQCdahuXMIZaERbuydx85894209 = GfQCdahuXMIZaERbuydx35634245;     GfQCdahuXMIZaERbuydx35634245 = GfQCdahuXMIZaERbuydx60137725;     GfQCdahuXMIZaERbuydx60137725 = GfQCdahuXMIZaERbuydx67069599;     GfQCdahuXMIZaERbuydx67069599 = GfQCdahuXMIZaERbuydx68920803;     GfQCdahuXMIZaERbuydx68920803 = GfQCdahuXMIZaERbuydx73494664;     GfQCdahuXMIZaERbuydx73494664 = GfQCdahuXMIZaERbuydx73136643;     GfQCdahuXMIZaERbuydx73136643 = GfQCdahuXMIZaERbuydx16147764;     GfQCdahuXMIZaERbuydx16147764 = GfQCdahuXMIZaERbuydx91085253;     GfQCdahuXMIZaERbuydx91085253 = GfQCdahuXMIZaERbuydx89498299;     GfQCdahuXMIZaERbuydx89498299 = GfQCdahuXMIZaERbuydx56133859;     GfQCdahuXMIZaERbuydx56133859 = GfQCdahuXMIZaERbuydx41483991;     GfQCdahuXMIZaERbuydx41483991 = GfQCdahuXMIZaERbuydx62211944;     GfQCdahuXMIZaERbuydx62211944 = GfQCdahuXMIZaERbuydx60034014;     GfQCdahuXMIZaERbuydx60034014 = GfQCdahuXMIZaERbuydx96089830;     GfQCdahuXMIZaERbuydx96089830 = GfQCdahuXMIZaERbuydx38753336;     GfQCdahuXMIZaERbuydx38753336 = GfQCdahuXMIZaERbuydx20874252;     GfQCdahuXMIZaERbuydx20874252 = GfQCdahuXMIZaERbuydx53774626;     GfQCdahuXMIZaERbuydx53774626 = GfQCdahuXMIZaERbuydx77405006;     GfQCdahuXMIZaERbuydx77405006 = GfQCdahuXMIZaERbuydx7709555;     GfQCdahuXMIZaERbuydx7709555 = GfQCdahuXMIZaERbuydx74100082;     GfQCdahuXMIZaERbuydx74100082 = GfQCdahuXMIZaERbuydx26055373;     GfQCdahuXMIZaERbuydx26055373 = GfQCdahuXMIZaERbuydx34596658;     GfQCdahuXMIZaERbuydx34596658 = GfQCdahuXMIZaERbuydx2640851;     GfQCdahuXMIZaERbuydx2640851 = GfQCdahuXMIZaERbuydx98178093;     GfQCdahuXMIZaERbuydx98178093 = GfQCdahuXMIZaERbuydx40705765;     GfQCdahuXMIZaERbuydx40705765 = GfQCdahuXMIZaERbuydx51709787;     GfQCdahuXMIZaERbuydx51709787 = GfQCdahuXMIZaERbuydx12716592;     GfQCdahuXMIZaERbuydx12716592 = GfQCdahuXMIZaERbuydx92711089;     GfQCdahuXMIZaERbuydx92711089 = GfQCdahuXMIZaERbuydx13271594;     GfQCdahuXMIZaERbuydx13271594 = GfQCdahuXMIZaERbuydx62629120;     GfQCdahuXMIZaERbuydx62629120 = GfQCdahuXMIZaERbuydx96742752;     GfQCdahuXMIZaERbuydx96742752 = GfQCdahuXMIZaERbuydx19775122;     GfQCdahuXMIZaERbuydx19775122 = GfQCdahuXMIZaERbuydx27660052;     GfQCdahuXMIZaERbuydx27660052 = GfQCdahuXMIZaERbuydx40821750;     GfQCdahuXMIZaERbuydx40821750 = GfQCdahuXMIZaERbuydx74698606;     GfQCdahuXMIZaERbuydx74698606 = GfQCdahuXMIZaERbuydx18411628;     GfQCdahuXMIZaERbuydx18411628 = GfQCdahuXMIZaERbuydx75239334;     GfQCdahuXMIZaERbuydx75239334 = GfQCdahuXMIZaERbuydx64100210;     GfQCdahuXMIZaERbuydx64100210 = GfQCdahuXMIZaERbuydx21794833;     GfQCdahuXMIZaERbuydx21794833 = GfQCdahuXMIZaERbuydx76281838;     GfQCdahuXMIZaERbuydx76281838 = GfQCdahuXMIZaERbuydx72612841;     GfQCdahuXMIZaERbuydx72612841 = GfQCdahuXMIZaERbuydx23807639;     GfQCdahuXMIZaERbuydx23807639 = GfQCdahuXMIZaERbuydx79560442;     GfQCdahuXMIZaERbuydx79560442 = GfQCdahuXMIZaERbuydx2694569;     GfQCdahuXMIZaERbuydx2694569 = GfQCdahuXMIZaERbuydx2813021;     GfQCdahuXMIZaERbuydx2813021 = GfQCdahuXMIZaERbuydx19329331;     GfQCdahuXMIZaERbuydx19329331 = GfQCdahuXMIZaERbuydx5829475;     GfQCdahuXMIZaERbuydx5829475 = GfQCdahuXMIZaERbuydx46461344;     GfQCdahuXMIZaERbuydx46461344 = GfQCdahuXMIZaERbuydx12438473;     GfQCdahuXMIZaERbuydx12438473 = GfQCdahuXMIZaERbuydx24584448;     GfQCdahuXMIZaERbuydx24584448 = GfQCdahuXMIZaERbuydx36223036;     GfQCdahuXMIZaERbuydx36223036 = GfQCdahuXMIZaERbuydx6169790;     GfQCdahuXMIZaERbuydx6169790 = GfQCdahuXMIZaERbuydx78421854;     GfQCdahuXMIZaERbuydx78421854 = GfQCdahuXMIZaERbuydx32015387;     GfQCdahuXMIZaERbuydx32015387 = GfQCdahuXMIZaERbuydx59838837;     GfQCdahuXMIZaERbuydx59838837 = GfQCdahuXMIZaERbuydx1037587;     GfQCdahuXMIZaERbuydx1037587 = GfQCdahuXMIZaERbuydx57496874;     GfQCdahuXMIZaERbuydx57496874 = GfQCdahuXMIZaERbuydx68891505;     GfQCdahuXMIZaERbuydx68891505 = GfQCdahuXMIZaERbuydx28215039;     GfQCdahuXMIZaERbuydx28215039 = GfQCdahuXMIZaERbuydx21784877;     GfQCdahuXMIZaERbuydx21784877 = GfQCdahuXMIZaERbuydx60420051;     GfQCdahuXMIZaERbuydx60420051 = GfQCdahuXMIZaERbuydx23436674;     GfQCdahuXMIZaERbuydx23436674 = GfQCdahuXMIZaERbuydx77813659;     GfQCdahuXMIZaERbuydx77813659 = GfQCdahuXMIZaERbuydx26869179;     GfQCdahuXMIZaERbuydx26869179 = GfQCdahuXMIZaERbuydx59391107;     GfQCdahuXMIZaERbuydx59391107 = GfQCdahuXMIZaERbuydx21708870;     GfQCdahuXMIZaERbuydx21708870 = GfQCdahuXMIZaERbuydx34551892;     GfQCdahuXMIZaERbuydx34551892 = GfQCdahuXMIZaERbuydx19212264;     GfQCdahuXMIZaERbuydx19212264 = GfQCdahuXMIZaERbuydx21391224;     GfQCdahuXMIZaERbuydx21391224 = GfQCdahuXMIZaERbuydx20341709;     GfQCdahuXMIZaERbuydx20341709 = GfQCdahuXMIZaERbuydx45634918;     GfQCdahuXMIZaERbuydx45634918 = GfQCdahuXMIZaERbuydx89674415;     GfQCdahuXMIZaERbuydx89674415 = GfQCdahuXMIZaERbuydx55610174;     GfQCdahuXMIZaERbuydx55610174 = GfQCdahuXMIZaERbuydx31427717;     GfQCdahuXMIZaERbuydx31427717 = GfQCdahuXMIZaERbuydx1487241;     GfQCdahuXMIZaERbuydx1487241 = GfQCdahuXMIZaERbuydx2247734;     GfQCdahuXMIZaERbuydx2247734 = GfQCdahuXMIZaERbuydx55036216;     GfQCdahuXMIZaERbuydx55036216 = GfQCdahuXMIZaERbuydx99946281;     GfQCdahuXMIZaERbuydx99946281 = GfQCdahuXMIZaERbuydx95365073;     GfQCdahuXMIZaERbuydx95365073 = GfQCdahuXMIZaERbuydx21376434;     GfQCdahuXMIZaERbuydx21376434 = GfQCdahuXMIZaERbuydx45880312;     GfQCdahuXMIZaERbuydx45880312 = GfQCdahuXMIZaERbuydx66255248;     GfQCdahuXMIZaERbuydx66255248 = GfQCdahuXMIZaERbuydx80272617;     GfQCdahuXMIZaERbuydx80272617 = GfQCdahuXMIZaERbuydx88687146;     GfQCdahuXMIZaERbuydx88687146 = GfQCdahuXMIZaERbuydx26406084;     GfQCdahuXMIZaERbuydx26406084 = GfQCdahuXMIZaERbuydx90572963;     GfQCdahuXMIZaERbuydx90572963 = GfQCdahuXMIZaERbuydx41353267;     GfQCdahuXMIZaERbuydx41353267 = GfQCdahuXMIZaERbuydx95644665;     GfQCdahuXMIZaERbuydx95644665 = GfQCdahuXMIZaERbuydx80982913;     GfQCdahuXMIZaERbuydx80982913 = GfQCdahuXMIZaERbuydx44297012;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lIvBDvbcoDOcFwcioSgJ86422685() {     double XKOEeXZQlqAAyLgEcLGN69882288 = -714761884;    double XKOEeXZQlqAAyLgEcLGN77938418 = -585415495;    double XKOEeXZQlqAAyLgEcLGN98742664 = -732635421;    double XKOEeXZQlqAAyLgEcLGN88273103 = -612586009;    double XKOEeXZQlqAAyLgEcLGN20533814 = 53187224;    double XKOEeXZQlqAAyLgEcLGN62467976 = -980501347;    double XKOEeXZQlqAAyLgEcLGN95318506 = 58666636;    double XKOEeXZQlqAAyLgEcLGN98584919 = -874748108;    double XKOEeXZQlqAAyLgEcLGN11460702 = -916620155;    double XKOEeXZQlqAAyLgEcLGN25273851 = -858223439;    double XKOEeXZQlqAAyLgEcLGN227764 = -418282227;    double XKOEeXZQlqAAyLgEcLGN70951900 = -511157084;    double XKOEeXZQlqAAyLgEcLGN36718503 = -911459039;    double XKOEeXZQlqAAyLgEcLGN58629316 = -698752976;    double XKOEeXZQlqAAyLgEcLGN25189691 = -975481561;    double XKOEeXZQlqAAyLgEcLGN81215707 = -567746648;    double XKOEeXZQlqAAyLgEcLGN72934759 = -176284971;    double XKOEeXZQlqAAyLgEcLGN37263668 = -960121592;    double XKOEeXZQlqAAyLgEcLGN52549975 = -166038339;    double XKOEeXZQlqAAyLgEcLGN50659766 = -638629769;    double XKOEeXZQlqAAyLgEcLGN57143125 = -828197642;    double XKOEeXZQlqAAyLgEcLGN42987098 = -913717849;    double XKOEeXZQlqAAyLgEcLGN65975742 = -411035587;    double XKOEeXZQlqAAyLgEcLGN61748056 = -50058223;    double XKOEeXZQlqAAyLgEcLGN50992970 = -492271488;    double XKOEeXZQlqAAyLgEcLGN68242473 = -829500331;    double XKOEeXZQlqAAyLgEcLGN3262162 = -630000303;    double XKOEeXZQlqAAyLgEcLGN17673426 = -754857612;    double XKOEeXZQlqAAyLgEcLGN85282219 = -255902203;    double XKOEeXZQlqAAyLgEcLGN61032045 = -903815512;    double XKOEeXZQlqAAyLgEcLGN96146716 = -21647457;    double XKOEeXZQlqAAyLgEcLGN63245305 = -389884204;    double XKOEeXZQlqAAyLgEcLGN55783710 = 80158359;    double XKOEeXZQlqAAyLgEcLGN56227922 = -635622119;    double XKOEeXZQlqAAyLgEcLGN99583081 = -76403668;    double XKOEeXZQlqAAyLgEcLGN45497615 = -863898198;    double XKOEeXZQlqAAyLgEcLGN1355173 = 51646035;    double XKOEeXZQlqAAyLgEcLGN88289278 = -257033136;    double XKOEeXZQlqAAyLgEcLGN33634669 = -846735923;    double XKOEeXZQlqAAyLgEcLGN22055268 = -198745095;    double XKOEeXZQlqAAyLgEcLGN9780401 = -999638388;    double XKOEeXZQlqAAyLgEcLGN40752343 = -890247206;    double XKOEeXZQlqAAyLgEcLGN68975341 = 97188101;    double XKOEeXZQlqAAyLgEcLGN98453331 = -845344587;    double XKOEeXZQlqAAyLgEcLGN32802627 = -782150975;    double XKOEeXZQlqAAyLgEcLGN41098929 = -19958590;    double XKOEeXZQlqAAyLgEcLGN18180082 = -380574752;    double XKOEeXZQlqAAyLgEcLGN83352105 = -740146434;    double XKOEeXZQlqAAyLgEcLGN14911530 = -129741835;    double XKOEeXZQlqAAyLgEcLGN75604827 = -740909376;    double XKOEeXZQlqAAyLgEcLGN86418085 = -370226505;    double XKOEeXZQlqAAyLgEcLGN99871420 = -737932350;    double XKOEeXZQlqAAyLgEcLGN41881243 = -529948133;    double XKOEeXZQlqAAyLgEcLGN48459351 = -598782070;    double XKOEeXZQlqAAyLgEcLGN29279561 = -117780512;    double XKOEeXZQlqAAyLgEcLGN26895191 = -801044035;    double XKOEeXZQlqAAyLgEcLGN11962677 = -74379909;    double XKOEeXZQlqAAyLgEcLGN36994608 = -582577198;    double XKOEeXZQlqAAyLgEcLGN37280134 = -20314522;    double XKOEeXZQlqAAyLgEcLGN52291341 = -117312445;    double XKOEeXZQlqAAyLgEcLGN59205815 = -250501045;    double XKOEeXZQlqAAyLgEcLGN77645080 = -186475752;    double XKOEeXZQlqAAyLgEcLGN13302701 = -518845906;    double XKOEeXZQlqAAyLgEcLGN50428656 = 87195357;    double XKOEeXZQlqAAyLgEcLGN29127135 = -736575982;    double XKOEeXZQlqAAyLgEcLGN36982459 = 71601977;    double XKOEeXZQlqAAyLgEcLGN15168191 = -491315444;    double XKOEeXZQlqAAyLgEcLGN80490581 = -175836920;    double XKOEeXZQlqAAyLgEcLGN59046234 = -522349309;    double XKOEeXZQlqAAyLgEcLGN79692076 = -11583363;    double XKOEeXZQlqAAyLgEcLGN79860534 = -519392683;    double XKOEeXZQlqAAyLgEcLGN84645480 = -919251835;    double XKOEeXZQlqAAyLgEcLGN3629000 = -13385670;    double XKOEeXZQlqAAyLgEcLGN30494707 = -967293244;    double XKOEeXZQlqAAyLgEcLGN40879365 = -638991381;    double XKOEeXZQlqAAyLgEcLGN16390782 = -937950437;    double XKOEeXZQlqAAyLgEcLGN74011757 = -910905951;    double XKOEeXZQlqAAyLgEcLGN67522411 = -565691000;    double XKOEeXZQlqAAyLgEcLGN28945430 = -267907248;    double XKOEeXZQlqAAyLgEcLGN9894041 = -372312898;    double XKOEeXZQlqAAyLgEcLGN50062392 = -348925580;    double XKOEeXZQlqAAyLgEcLGN19910056 = -889853870;    double XKOEeXZQlqAAyLgEcLGN2761896 = -525115778;    double XKOEeXZQlqAAyLgEcLGN9677392 = -514992828;    double XKOEeXZQlqAAyLgEcLGN74613960 = -433589008;    double XKOEeXZQlqAAyLgEcLGN96275295 = -283715108;    double XKOEeXZQlqAAyLgEcLGN21364063 = -859936072;    double XKOEeXZQlqAAyLgEcLGN7324359 = -321059571;    double XKOEeXZQlqAAyLgEcLGN26948362 = -417841607;    double XKOEeXZQlqAAyLgEcLGN72687891 = -275359633;    double XKOEeXZQlqAAyLgEcLGN33534939 = -689518290;    double XKOEeXZQlqAAyLgEcLGN64360564 = -365776767;    double XKOEeXZQlqAAyLgEcLGN51009145 = -136718615;    double XKOEeXZQlqAAyLgEcLGN81343327 = -629423479;    double XKOEeXZQlqAAyLgEcLGN62849452 = -948244051;    double XKOEeXZQlqAAyLgEcLGN32135321 = -713162637;    double XKOEeXZQlqAAyLgEcLGN27449642 = -271401301;    double XKOEeXZQlqAAyLgEcLGN18546685 = -990007256;    double XKOEeXZQlqAAyLgEcLGN69326196 = -8768605;    double XKOEeXZQlqAAyLgEcLGN95820167 = -714761884;     XKOEeXZQlqAAyLgEcLGN69882288 = XKOEeXZQlqAAyLgEcLGN77938418;     XKOEeXZQlqAAyLgEcLGN77938418 = XKOEeXZQlqAAyLgEcLGN98742664;     XKOEeXZQlqAAyLgEcLGN98742664 = XKOEeXZQlqAAyLgEcLGN88273103;     XKOEeXZQlqAAyLgEcLGN88273103 = XKOEeXZQlqAAyLgEcLGN20533814;     XKOEeXZQlqAAyLgEcLGN20533814 = XKOEeXZQlqAAyLgEcLGN62467976;     XKOEeXZQlqAAyLgEcLGN62467976 = XKOEeXZQlqAAyLgEcLGN95318506;     XKOEeXZQlqAAyLgEcLGN95318506 = XKOEeXZQlqAAyLgEcLGN98584919;     XKOEeXZQlqAAyLgEcLGN98584919 = XKOEeXZQlqAAyLgEcLGN11460702;     XKOEeXZQlqAAyLgEcLGN11460702 = XKOEeXZQlqAAyLgEcLGN25273851;     XKOEeXZQlqAAyLgEcLGN25273851 = XKOEeXZQlqAAyLgEcLGN227764;     XKOEeXZQlqAAyLgEcLGN227764 = XKOEeXZQlqAAyLgEcLGN70951900;     XKOEeXZQlqAAyLgEcLGN70951900 = XKOEeXZQlqAAyLgEcLGN36718503;     XKOEeXZQlqAAyLgEcLGN36718503 = XKOEeXZQlqAAyLgEcLGN58629316;     XKOEeXZQlqAAyLgEcLGN58629316 = XKOEeXZQlqAAyLgEcLGN25189691;     XKOEeXZQlqAAyLgEcLGN25189691 = XKOEeXZQlqAAyLgEcLGN81215707;     XKOEeXZQlqAAyLgEcLGN81215707 = XKOEeXZQlqAAyLgEcLGN72934759;     XKOEeXZQlqAAyLgEcLGN72934759 = XKOEeXZQlqAAyLgEcLGN37263668;     XKOEeXZQlqAAyLgEcLGN37263668 = XKOEeXZQlqAAyLgEcLGN52549975;     XKOEeXZQlqAAyLgEcLGN52549975 = XKOEeXZQlqAAyLgEcLGN50659766;     XKOEeXZQlqAAyLgEcLGN50659766 = XKOEeXZQlqAAyLgEcLGN57143125;     XKOEeXZQlqAAyLgEcLGN57143125 = XKOEeXZQlqAAyLgEcLGN42987098;     XKOEeXZQlqAAyLgEcLGN42987098 = XKOEeXZQlqAAyLgEcLGN65975742;     XKOEeXZQlqAAyLgEcLGN65975742 = XKOEeXZQlqAAyLgEcLGN61748056;     XKOEeXZQlqAAyLgEcLGN61748056 = XKOEeXZQlqAAyLgEcLGN50992970;     XKOEeXZQlqAAyLgEcLGN50992970 = XKOEeXZQlqAAyLgEcLGN68242473;     XKOEeXZQlqAAyLgEcLGN68242473 = XKOEeXZQlqAAyLgEcLGN3262162;     XKOEeXZQlqAAyLgEcLGN3262162 = XKOEeXZQlqAAyLgEcLGN17673426;     XKOEeXZQlqAAyLgEcLGN17673426 = XKOEeXZQlqAAyLgEcLGN85282219;     XKOEeXZQlqAAyLgEcLGN85282219 = XKOEeXZQlqAAyLgEcLGN61032045;     XKOEeXZQlqAAyLgEcLGN61032045 = XKOEeXZQlqAAyLgEcLGN96146716;     XKOEeXZQlqAAyLgEcLGN96146716 = XKOEeXZQlqAAyLgEcLGN63245305;     XKOEeXZQlqAAyLgEcLGN63245305 = XKOEeXZQlqAAyLgEcLGN55783710;     XKOEeXZQlqAAyLgEcLGN55783710 = XKOEeXZQlqAAyLgEcLGN56227922;     XKOEeXZQlqAAyLgEcLGN56227922 = XKOEeXZQlqAAyLgEcLGN99583081;     XKOEeXZQlqAAyLgEcLGN99583081 = XKOEeXZQlqAAyLgEcLGN45497615;     XKOEeXZQlqAAyLgEcLGN45497615 = XKOEeXZQlqAAyLgEcLGN1355173;     XKOEeXZQlqAAyLgEcLGN1355173 = XKOEeXZQlqAAyLgEcLGN88289278;     XKOEeXZQlqAAyLgEcLGN88289278 = XKOEeXZQlqAAyLgEcLGN33634669;     XKOEeXZQlqAAyLgEcLGN33634669 = XKOEeXZQlqAAyLgEcLGN22055268;     XKOEeXZQlqAAyLgEcLGN22055268 = XKOEeXZQlqAAyLgEcLGN9780401;     XKOEeXZQlqAAyLgEcLGN9780401 = XKOEeXZQlqAAyLgEcLGN40752343;     XKOEeXZQlqAAyLgEcLGN40752343 = XKOEeXZQlqAAyLgEcLGN68975341;     XKOEeXZQlqAAyLgEcLGN68975341 = XKOEeXZQlqAAyLgEcLGN98453331;     XKOEeXZQlqAAyLgEcLGN98453331 = XKOEeXZQlqAAyLgEcLGN32802627;     XKOEeXZQlqAAyLgEcLGN32802627 = XKOEeXZQlqAAyLgEcLGN41098929;     XKOEeXZQlqAAyLgEcLGN41098929 = XKOEeXZQlqAAyLgEcLGN18180082;     XKOEeXZQlqAAyLgEcLGN18180082 = XKOEeXZQlqAAyLgEcLGN83352105;     XKOEeXZQlqAAyLgEcLGN83352105 = XKOEeXZQlqAAyLgEcLGN14911530;     XKOEeXZQlqAAyLgEcLGN14911530 = XKOEeXZQlqAAyLgEcLGN75604827;     XKOEeXZQlqAAyLgEcLGN75604827 = XKOEeXZQlqAAyLgEcLGN86418085;     XKOEeXZQlqAAyLgEcLGN86418085 = XKOEeXZQlqAAyLgEcLGN99871420;     XKOEeXZQlqAAyLgEcLGN99871420 = XKOEeXZQlqAAyLgEcLGN41881243;     XKOEeXZQlqAAyLgEcLGN41881243 = XKOEeXZQlqAAyLgEcLGN48459351;     XKOEeXZQlqAAyLgEcLGN48459351 = XKOEeXZQlqAAyLgEcLGN29279561;     XKOEeXZQlqAAyLgEcLGN29279561 = XKOEeXZQlqAAyLgEcLGN26895191;     XKOEeXZQlqAAyLgEcLGN26895191 = XKOEeXZQlqAAyLgEcLGN11962677;     XKOEeXZQlqAAyLgEcLGN11962677 = XKOEeXZQlqAAyLgEcLGN36994608;     XKOEeXZQlqAAyLgEcLGN36994608 = XKOEeXZQlqAAyLgEcLGN37280134;     XKOEeXZQlqAAyLgEcLGN37280134 = XKOEeXZQlqAAyLgEcLGN52291341;     XKOEeXZQlqAAyLgEcLGN52291341 = XKOEeXZQlqAAyLgEcLGN59205815;     XKOEeXZQlqAAyLgEcLGN59205815 = XKOEeXZQlqAAyLgEcLGN77645080;     XKOEeXZQlqAAyLgEcLGN77645080 = XKOEeXZQlqAAyLgEcLGN13302701;     XKOEeXZQlqAAyLgEcLGN13302701 = XKOEeXZQlqAAyLgEcLGN50428656;     XKOEeXZQlqAAyLgEcLGN50428656 = XKOEeXZQlqAAyLgEcLGN29127135;     XKOEeXZQlqAAyLgEcLGN29127135 = XKOEeXZQlqAAyLgEcLGN36982459;     XKOEeXZQlqAAyLgEcLGN36982459 = XKOEeXZQlqAAyLgEcLGN15168191;     XKOEeXZQlqAAyLgEcLGN15168191 = XKOEeXZQlqAAyLgEcLGN80490581;     XKOEeXZQlqAAyLgEcLGN80490581 = XKOEeXZQlqAAyLgEcLGN59046234;     XKOEeXZQlqAAyLgEcLGN59046234 = XKOEeXZQlqAAyLgEcLGN79692076;     XKOEeXZQlqAAyLgEcLGN79692076 = XKOEeXZQlqAAyLgEcLGN79860534;     XKOEeXZQlqAAyLgEcLGN79860534 = XKOEeXZQlqAAyLgEcLGN84645480;     XKOEeXZQlqAAyLgEcLGN84645480 = XKOEeXZQlqAAyLgEcLGN3629000;     XKOEeXZQlqAAyLgEcLGN3629000 = XKOEeXZQlqAAyLgEcLGN30494707;     XKOEeXZQlqAAyLgEcLGN30494707 = XKOEeXZQlqAAyLgEcLGN40879365;     XKOEeXZQlqAAyLgEcLGN40879365 = XKOEeXZQlqAAyLgEcLGN16390782;     XKOEeXZQlqAAyLgEcLGN16390782 = XKOEeXZQlqAAyLgEcLGN74011757;     XKOEeXZQlqAAyLgEcLGN74011757 = XKOEeXZQlqAAyLgEcLGN67522411;     XKOEeXZQlqAAyLgEcLGN67522411 = XKOEeXZQlqAAyLgEcLGN28945430;     XKOEeXZQlqAAyLgEcLGN28945430 = XKOEeXZQlqAAyLgEcLGN9894041;     XKOEeXZQlqAAyLgEcLGN9894041 = XKOEeXZQlqAAyLgEcLGN50062392;     XKOEeXZQlqAAyLgEcLGN50062392 = XKOEeXZQlqAAyLgEcLGN19910056;     XKOEeXZQlqAAyLgEcLGN19910056 = XKOEeXZQlqAAyLgEcLGN2761896;     XKOEeXZQlqAAyLgEcLGN2761896 = XKOEeXZQlqAAyLgEcLGN9677392;     XKOEeXZQlqAAyLgEcLGN9677392 = XKOEeXZQlqAAyLgEcLGN74613960;     XKOEeXZQlqAAyLgEcLGN74613960 = XKOEeXZQlqAAyLgEcLGN96275295;     XKOEeXZQlqAAyLgEcLGN96275295 = XKOEeXZQlqAAyLgEcLGN21364063;     XKOEeXZQlqAAyLgEcLGN21364063 = XKOEeXZQlqAAyLgEcLGN7324359;     XKOEeXZQlqAAyLgEcLGN7324359 = XKOEeXZQlqAAyLgEcLGN26948362;     XKOEeXZQlqAAyLgEcLGN26948362 = XKOEeXZQlqAAyLgEcLGN72687891;     XKOEeXZQlqAAyLgEcLGN72687891 = XKOEeXZQlqAAyLgEcLGN33534939;     XKOEeXZQlqAAyLgEcLGN33534939 = XKOEeXZQlqAAyLgEcLGN64360564;     XKOEeXZQlqAAyLgEcLGN64360564 = XKOEeXZQlqAAyLgEcLGN51009145;     XKOEeXZQlqAAyLgEcLGN51009145 = XKOEeXZQlqAAyLgEcLGN81343327;     XKOEeXZQlqAAyLgEcLGN81343327 = XKOEeXZQlqAAyLgEcLGN62849452;     XKOEeXZQlqAAyLgEcLGN62849452 = XKOEeXZQlqAAyLgEcLGN32135321;     XKOEeXZQlqAAyLgEcLGN32135321 = XKOEeXZQlqAAyLgEcLGN27449642;     XKOEeXZQlqAAyLgEcLGN27449642 = XKOEeXZQlqAAyLgEcLGN18546685;     XKOEeXZQlqAAyLgEcLGN18546685 = XKOEeXZQlqAAyLgEcLGN69326196;     XKOEeXZQlqAAyLgEcLGN69326196 = XKOEeXZQlqAAyLgEcLGN95820167;     XKOEeXZQlqAAyLgEcLGN95820167 = XKOEeXZQlqAAyLgEcLGN69882288;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TfmwYGluyRNNexnvYytY14016942() {     double GFuBFiTyybWUxkMolqNx95467565 = -139794984;    double GFuBFiTyybWUxkMolqNx74335561 = -840893272;    double GFuBFiTyybWUxkMolqNx31621840 = -535096957;    double GFuBFiTyybWUxkMolqNx33995033 = -619908902;    double GFuBFiTyybWUxkMolqNx89875818 = 6264937;    double GFuBFiTyybWUxkMolqNx79477253 = -35482351;    double GFuBFiTyybWUxkMolqNx639351 = -523604571;    double GFuBFiTyybWUxkMolqNx13595044 = -155429987;    double GFuBFiTyybWUxkMolqNx36789995 = -312844200;    double GFuBFiTyybWUxkMolqNx44432233 = -92784962;    double GFuBFiTyybWUxkMolqNx14561318 = -728710113;    double GFuBFiTyybWUxkMolqNx6269557 = -954865991;    double GFuBFiTyybWUxkMolqNx13299281 = -675707404;    double GFuBFiTyybWUxkMolqNx50189033 = -673277269;    double GFuBFiTyybWUxkMolqNx81458579 = -864027519;    double GFuBFiTyybWUxkMolqNx88936750 = 96061875;    double GFuBFiTyybWUxkMolqNx72732876 = -964604765;    double GFuBFiTyybWUxkMolqNx58379572 = -376398953;    double GFuBFiTyybWUxkMolqNx14014696 = -949012196;    double GFuBFiTyybWUxkMolqNx11821233 = -613069144;    double GFuBFiTyybWUxkMolqNx58152390 = -189897424;    double GFuBFiTyybWUxkMolqNx44490205 = -233450779;    double GFuBFiTyybWUxkMolqNx69739540 = -455127692;    double GFuBFiTyybWUxkMolqNx63462099 = 91092798;    double GFuBFiTyybWUxkMolqNx5896109 = -59767630;    double GFuBFiTyybWUxkMolqNx97731609 = -393950873;    double GFuBFiTyybWUxkMolqNx85650070 = -195813438;    double GFuBFiTyybWUxkMolqNx81572226 = -984519845;    double GFuBFiTyybWUxkMolqNx93159431 = -492196054;    double GFuBFiTyybWUxkMolqNx14354537 = -812725074;    double GFuBFiTyybWUxkMolqNx18193351 = -900294488;    double GFuBFiTyybWUxkMolqNx435238 = -663680038;    double GFuBFiTyybWUxkMolqNx76970761 = -367662778;    double GFuBFiTyybWUxkMolqNx9814993 = -691767090;    double GFuBFiTyybWUxkMolqNx988070 = -777839579;    double GFuBFiTyybWUxkMolqNx50289465 = -281492914;    double GFuBFiTyybWUxkMolqNx51000558 = -428865909;    double GFuBFiTyybWUxkMolqNx63861965 = -105747961;    double GFuBFiTyybWUxkMolqNx74558247 = -205074245;    double GFuBFiTyybWUxkMolqNx30838942 = -668715415;    double GFuBFiTyybWUxkMolqNx56931682 = -474698578;    double GFuBFiTyybWUxkMolqNx84761932 = -299285710;    double GFuBFiTyybWUxkMolqNx18175561 = 68581720;    double GFuBFiTyybWUxkMolqNx69246610 = -213760186;    double GFuBFiTyybWUxkMolqNx24783503 = -721172093;    double GFuBFiTyybWUxkMolqNx7499251 = -542506128;    double GFuBFiTyybWUxkMolqNx17948536 = -413043612;    double GFuBFiTyybWUxkMolqNx91464876 = -933329281;    double GFuBFiTyybWUxkMolqNx65722850 = -229418061;    double GFuBFiTyybWUxkMolqNx29414823 = -588932529;    double GFuBFiTyybWUxkMolqNx96554333 = -618885107;    double GFuBFiTyybWUxkMolqNx27130001 = -916011529;    double GFuBFiTyybWUxkMolqNx59954846 = 2153319;    double GFuBFiTyybWUxkMolqNx17358260 = -913845852;    double GFuBFiTyybWUxkMolqNx55864552 = -592870930;    double GFuBFiTyybWUxkMolqNx50977361 = -906344206;    double GFuBFiTyybWUxkMolqNx4596022 = -285765580;    double GFuBFiTyybWUxkMolqNx68159741 = -526189755;    double GFuBFiTyybWUxkMolqNx28098925 = -460141273;    double GFuBFiTyybWUxkMolqNx92144208 = -599784191;    double GFuBFiTyybWUxkMolqNx93827182 = -839668913;    double GFuBFiTyybWUxkMolqNx19067125 = -539084726;    double GFuBFiTyybWUxkMolqNx20435613 = -663233934;    double GFuBFiTyybWUxkMolqNx22435458 = -500119126;    double GFuBFiTyybWUxkMolqNx26238883 = -192490474;    double GFuBFiTyybWUxkMolqNx14126080 = 34969925;    double GFuBFiTyybWUxkMolqNx29298795 = -487203213;    double GFuBFiTyybWUxkMolqNx3484288 = -983940314;    double GFuBFiTyybWUxkMolqNx49200963 = -895437691;    double GFuBFiTyybWUxkMolqNx31169114 = -482534605;    double GFuBFiTyybWUxkMolqNx37936192 = -475072216;    double GFuBFiTyybWUxkMolqNx8870911 = -758856804;    double GFuBFiTyybWUxkMolqNx83821325 = -71324709;    double GFuBFiTyybWUxkMolqNx83175754 = -180296782;    double GFuBFiTyybWUxkMolqNx54889550 = -38370566;    double GFuBFiTyybWUxkMolqNx73390457 = -890611714;    double GFuBFiTyybWUxkMolqNx26314644 = -202032499;    double GFuBFiTyybWUxkMolqNx492930 = -141367507;    double GFuBFiTyybWUxkMolqNx38678596 = -187735110;    double GFuBFiTyybWUxkMolqNx98396857 = -517261503;    double GFuBFiTyybWUxkMolqNx79783074 = -980907261;    double GFuBFiTyybWUxkMolqNx94185194 = -262484158;    double GFuBFiTyybWUxkMolqNx15849376 = -655101784;    double GFuBFiTyybWUxkMolqNx63744608 = -903263525;    double GFuBFiTyybWUxkMolqNx17800204 = -93839968;    double GFuBFiTyybWUxkMolqNx91063350 = -984282959;    double GFuBFiTyybWUxkMolqNx40480391 = -565833357;    double GFuBFiTyybWUxkMolqNx59612502 = -453816927;    double GFuBFiTyybWUxkMolqNx53950441 = 1103839;    double GFuBFiTyybWUxkMolqNx50010709 = -871495374;    double GFuBFiTyybWUxkMolqNx45693444 = -995727334;    double GFuBFiTyybWUxkMolqNx82840816 = -902676155;    double GFuBFiTyybWUxkMolqNx35763041 = -645606689;    double GFuBFiTyybWUxkMolqNx82414038 = -605290055;    double GFuBFiTyybWUxkMolqNx37011759 = -829046502;    double GFuBFiTyybWUxkMolqNx37864557 = -935613852;    double GFuBFiTyybWUxkMolqNx64326320 = -636051777;    double GFuBFiTyybWUxkMolqNx95740102 = -431299155;    double GFuBFiTyybWUxkMolqNx43007728 = 78730288;    double GFuBFiTyybWUxkMolqNx10657423 = -139794984;     GFuBFiTyybWUxkMolqNx95467565 = GFuBFiTyybWUxkMolqNx74335561;     GFuBFiTyybWUxkMolqNx74335561 = GFuBFiTyybWUxkMolqNx31621840;     GFuBFiTyybWUxkMolqNx31621840 = GFuBFiTyybWUxkMolqNx33995033;     GFuBFiTyybWUxkMolqNx33995033 = GFuBFiTyybWUxkMolqNx89875818;     GFuBFiTyybWUxkMolqNx89875818 = GFuBFiTyybWUxkMolqNx79477253;     GFuBFiTyybWUxkMolqNx79477253 = GFuBFiTyybWUxkMolqNx639351;     GFuBFiTyybWUxkMolqNx639351 = GFuBFiTyybWUxkMolqNx13595044;     GFuBFiTyybWUxkMolqNx13595044 = GFuBFiTyybWUxkMolqNx36789995;     GFuBFiTyybWUxkMolqNx36789995 = GFuBFiTyybWUxkMolqNx44432233;     GFuBFiTyybWUxkMolqNx44432233 = GFuBFiTyybWUxkMolqNx14561318;     GFuBFiTyybWUxkMolqNx14561318 = GFuBFiTyybWUxkMolqNx6269557;     GFuBFiTyybWUxkMolqNx6269557 = GFuBFiTyybWUxkMolqNx13299281;     GFuBFiTyybWUxkMolqNx13299281 = GFuBFiTyybWUxkMolqNx50189033;     GFuBFiTyybWUxkMolqNx50189033 = GFuBFiTyybWUxkMolqNx81458579;     GFuBFiTyybWUxkMolqNx81458579 = GFuBFiTyybWUxkMolqNx88936750;     GFuBFiTyybWUxkMolqNx88936750 = GFuBFiTyybWUxkMolqNx72732876;     GFuBFiTyybWUxkMolqNx72732876 = GFuBFiTyybWUxkMolqNx58379572;     GFuBFiTyybWUxkMolqNx58379572 = GFuBFiTyybWUxkMolqNx14014696;     GFuBFiTyybWUxkMolqNx14014696 = GFuBFiTyybWUxkMolqNx11821233;     GFuBFiTyybWUxkMolqNx11821233 = GFuBFiTyybWUxkMolqNx58152390;     GFuBFiTyybWUxkMolqNx58152390 = GFuBFiTyybWUxkMolqNx44490205;     GFuBFiTyybWUxkMolqNx44490205 = GFuBFiTyybWUxkMolqNx69739540;     GFuBFiTyybWUxkMolqNx69739540 = GFuBFiTyybWUxkMolqNx63462099;     GFuBFiTyybWUxkMolqNx63462099 = GFuBFiTyybWUxkMolqNx5896109;     GFuBFiTyybWUxkMolqNx5896109 = GFuBFiTyybWUxkMolqNx97731609;     GFuBFiTyybWUxkMolqNx97731609 = GFuBFiTyybWUxkMolqNx85650070;     GFuBFiTyybWUxkMolqNx85650070 = GFuBFiTyybWUxkMolqNx81572226;     GFuBFiTyybWUxkMolqNx81572226 = GFuBFiTyybWUxkMolqNx93159431;     GFuBFiTyybWUxkMolqNx93159431 = GFuBFiTyybWUxkMolqNx14354537;     GFuBFiTyybWUxkMolqNx14354537 = GFuBFiTyybWUxkMolqNx18193351;     GFuBFiTyybWUxkMolqNx18193351 = GFuBFiTyybWUxkMolqNx435238;     GFuBFiTyybWUxkMolqNx435238 = GFuBFiTyybWUxkMolqNx76970761;     GFuBFiTyybWUxkMolqNx76970761 = GFuBFiTyybWUxkMolqNx9814993;     GFuBFiTyybWUxkMolqNx9814993 = GFuBFiTyybWUxkMolqNx988070;     GFuBFiTyybWUxkMolqNx988070 = GFuBFiTyybWUxkMolqNx50289465;     GFuBFiTyybWUxkMolqNx50289465 = GFuBFiTyybWUxkMolqNx51000558;     GFuBFiTyybWUxkMolqNx51000558 = GFuBFiTyybWUxkMolqNx63861965;     GFuBFiTyybWUxkMolqNx63861965 = GFuBFiTyybWUxkMolqNx74558247;     GFuBFiTyybWUxkMolqNx74558247 = GFuBFiTyybWUxkMolqNx30838942;     GFuBFiTyybWUxkMolqNx30838942 = GFuBFiTyybWUxkMolqNx56931682;     GFuBFiTyybWUxkMolqNx56931682 = GFuBFiTyybWUxkMolqNx84761932;     GFuBFiTyybWUxkMolqNx84761932 = GFuBFiTyybWUxkMolqNx18175561;     GFuBFiTyybWUxkMolqNx18175561 = GFuBFiTyybWUxkMolqNx69246610;     GFuBFiTyybWUxkMolqNx69246610 = GFuBFiTyybWUxkMolqNx24783503;     GFuBFiTyybWUxkMolqNx24783503 = GFuBFiTyybWUxkMolqNx7499251;     GFuBFiTyybWUxkMolqNx7499251 = GFuBFiTyybWUxkMolqNx17948536;     GFuBFiTyybWUxkMolqNx17948536 = GFuBFiTyybWUxkMolqNx91464876;     GFuBFiTyybWUxkMolqNx91464876 = GFuBFiTyybWUxkMolqNx65722850;     GFuBFiTyybWUxkMolqNx65722850 = GFuBFiTyybWUxkMolqNx29414823;     GFuBFiTyybWUxkMolqNx29414823 = GFuBFiTyybWUxkMolqNx96554333;     GFuBFiTyybWUxkMolqNx96554333 = GFuBFiTyybWUxkMolqNx27130001;     GFuBFiTyybWUxkMolqNx27130001 = GFuBFiTyybWUxkMolqNx59954846;     GFuBFiTyybWUxkMolqNx59954846 = GFuBFiTyybWUxkMolqNx17358260;     GFuBFiTyybWUxkMolqNx17358260 = GFuBFiTyybWUxkMolqNx55864552;     GFuBFiTyybWUxkMolqNx55864552 = GFuBFiTyybWUxkMolqNx50977361;     GFuBFiTyybWUxkMolqNx50977361 = GFuBFiTyybWUxkMolqNx4596022;     GFuBFiTyybWUxkMolqNx4596022 = GFuBFiTyybWUxkMolqNx68159741;     GFuBFiTyybWUxkMolqNx68159741 = GFuBFiTyybWUxkMolqNx28098925;     GFuBFiTyybWUxkMolqNx28098925 = GFuBFiTyybWUxkMolqNx92144208;     GFuBFiTyybWUxkMolqNx92144208 = GFuBFiTyybWUxkMolqNx93827182;     GFuBFiTyybWUxkMolqNx93827182 = GFuBFiTyybWUxkMolqNx19067125;     GFuBFiTyybWUxkMolqNx19067125 = GFuBFiTyybWUxkMolqNx20435613;     GFuBFiTyybWUxkMolqNx20435613 = GFuBFiTyybWUxkMolqNx22435458;     GFuBFiTyybWUxkMolqNx22435458 = GFuBFiTyybWUxkMolqNx26238883;     GFuBFiTyybWUxkMolqNx26238883 = GFuBFiTyybWUxkMolqNx14126080;     GFuBFiTyybWUxkMolqNx14126080 = GFuBFiTyybWUxkMolqNx29298795;     GFuBFiTyybWUxkMolqNx29298795 = GFuBFiTyybWUxkMolqNx3484288;     GFuBFiTyybWUxkMolqNx3484288 = GFuBFiTyybWUxkMolqNx49200963;     GFuBFiTyybWUxkMolqNx49200963 = GFuBFiTyybWUxkMolqNx31169114;     GFuBFiTyybWUxkMolqNx31169114 = GFuBFiTyybWUxkMolqNx37936192;     GFuBFiTyybWUxkMolqNx37936192 = GFuBFiTyybWUxkMolqNx8870911;     GFuBFiTyybWUxkMolqNx8870911 = GFuBFiTyybWUxkMolqNx83821325;     GFuBFiTyybWUxkMolqNx83821325 = GFuBFiTyybWUxkMolqNx83175754;     GFuBFiTyybWUxkMolqNx83175754 = GFuBFiTyybWUxkMolqNx54889550;     GFuBFiTyybWUxkMolqNx54889550 = GFuBFiTyybWUxkMolqNx73390457;     GFuBFiTyybWUxkMolqNx73390457 = GFuBFiTyybWUxkMolqNx26314644;     GFuBFiTyybWUxkMolqNx26314644 = GFuBFiTyybWUxkMolqNx492930;     GFuBFiTyybWUxkMolqNx492930 = GFuBFiTyybWUxkMolqNx38678596;     GFuBFiTyybWUxkMolqNx38678596 = GFuBFiTyybWUxkMolqNx98396857;     GFuBFiTyybWUxkMolqNx98396857 = GFuBFiTyybWUxkMolqNx79783074;     GFuBFiTyybWUxkMolqNx79783074 = GFuBFiTyybWUxkMolqNx94185194;     GFuBFiTyybWUxkMolqNx94185194 = GFuBFiTyybWUxkMolqNx15849376;     GFuBFiTyybWUxkMolqNx15849376 = GFuBFiTyybWUxkMolqNx63744608;     GFuBFiTyybWUxkMolqNx63744608 = GFuBFiTyybWUxkMolqNx17800204;     GFuBFiTyybWUxkMolqNx17800204 = GFuBFiTyybWUxkMolqNx91063350;     GFuBFiTyybWUxkMolqNx91063350 = GFuBFiTyybWUxkMolqNx40480391;     GFuBFiTyybWUxkMolqNx40480391 = GFuBFiTyybWUxkMolqNx59612502;     GFuBFiTyybWUxkMolqNx59612502 = GFuBFiTyybWUxkMolqNx53950441;     GFuBFiTyybWUxkMolqNx53950441 = GFuBFiTyybWUxkMolqNx50010709;     GFuBFiTyybWUxkMolqNx50010709 = GFuBFiTyybWUxkMolqNx45693444;     GFuBFiTyybWUxkMolqNx45693444 = GFuBFiTyybWUxkMolqNx82840816;     GFuBFiTyybWUxkMolqNx82840816 = GFuBFiTyybWUxkMolqNx35763041;     GFuBFiTyybWUxkMolqNx35763041 = GFuBFiTyybWUxkMolqNx82414038;     GFuBFiTyybWUxkMolqNx82414038 = GFuBFiTyybWUxkMolqNx37011759;     GFuBFiTyybWUxkMolqNx37011759 = GFuBFiTyybWUxkMolqNx37864557;     GFuBFiTyybWUxkMolqNx37864557 = GFuBFiTyybWUxkMolqNx64326320;     GFuBFiTyybWUxkMolqNx64326320 = GFuBFiTyybWUxkMolqNx95740102;     GFuBFiTyybWUxkMolqNx95740102 = GFuBFiTyybWUxkMolqNx43007728;     GFuBFiTyybWUxkMolqNx43007728 = GFuBFiTyybWUxkMolqNx10657423;     GFuBFiTyybWUxkMolqNx10657423 = GFuBFiTyybWUxkMolqNx95467565;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CRbCIPUqWuYselaRgfMG41611199() {     double sJsgBKKQVnLBFXUHltiH21052843 = -664828085;    double sJsgBKKQVnLBFXUHltiH70732705 = 3628952;    double sJsgBKKQVnLBFXUHltiH64501016 = -337558493;    double sJsgBKKQVnLBFXUHltiH79716963 = -627231796;    double sJsgBKKQVnLBFXUHltiH59217823 = -40657350;    double sJsgBKKQVnLBFXUHltiH96486529 = -190463354;    double sJsgBKKQVnLBFXUHltiH5960196 = -5875778;    double sJsgBKKQVnLBFXUHltiH28605168 = -536111867;    double sJsgBKKQVnLBFXUHltiH62119287 = -809068244;    double sJsgBKKQVnLBFXUHltiH63590616 = -427346486;    double sJsgBKKQVnLBFXUHltiH28894871 = 60862000;    double sJsgBKKQVnLBFXUHltiH41587212 = -298574898;    double sJsgBKKQVnLBFXUHltiH89880058 = -439955769;    double sJsgBKKQVnLBFXUHltiH41748750 = -647801563;    double sJsgBKKQVnLBFXUHltiH37727467 = -752573477;    double sJsgBKKQVnLBFXUHltiH96657793 = -340129602;    double sJsgBKKQVnLBFXUHltiH72530992 = -652924560;    double sJsgBKKQVnLBFXUHltiH79495476 = -892676314;    double sJsgBKKQVnLBFXUHltiH75479416 = -631986053;    double sJsgBKKQVnLBFXUHltiH72982699 = -587508520;    double sJsgBKKQVnLBFXUHltiH59161655 = -651597205;    double sJsgBKKQVnLBFXUHltiH45993311 = -653183708;    double sJsgBKKQVnLBFXUHltiH73503338 = -499219797;    double sJsgBKKQVnLBFXUHltiH65176141 = -867756182;    double sJsgBKKQVnLBFXUHltiH60799248 = -727263773;    double sJsgBKKQVnLBFXUHltiH27220746 = 41598586;    double sJsgBKKQVnLBFXUHltiH68037979 = -861626572;    double sJsgBKKQVnLBFXUHltiH45471026 = -114182077;    double sJsgBKKQVnLBFXUHltiH1036644 = -728489904;    double sJsgBKKQVnLBFXUHltiH67677027 = -721634636;    double sJsgBKKQVnLBFXUHltiH40239985 = -678941519;    double sJsgBKKQVnLBFXUHltiH37625170 = -937475872;    double sJsgBKKQVnLBFXUHltiH98157813 = -815483915;    double sJsgBKKQVnLBFXUHltiH63402064 = -747912062;    double sJsgBKKQVnLBFXUHltiH2393058 = -379275491;    double sJsgBKKQVnLBFXUHltiH55081316 = -799087630;    double sJsgBKKQVnLBFXUHltiH645944 = -909377854;    double sJsgBKKQVnLBFXUHltiH39434652 = 45537214;    double sJsgBKKQVnLBFXUHltiH15481826 = -663412567;    double sJsgBKKQVnLBFXUHltiH39622616 = -38685734;    double sJsgBKKQVnLBFXUHltiH4082963 = 50241232;    double sJsgBKKQVnLBFXUHltiH28771523 = -808324214;    double sJsgBKKQVnLBFXUHltiH67375780 = 39975339;    double sJsgBKKQVnLBFXUHltiH40039890 = -682175785;    double sJsgBKKQVnLBFXUHltiH16764379 = -660193210;    double sJsgBKKQVnLBFXUHltiH73899573 = 34946334;    double sJsgBKKQVnLBFXUHltiH17716990 = -445512473;    double sJsgBKKQVnLBFXUHltiH99577647 = -26512127;    double sJsgBKKQVnLBFXUHltiH16534170 = -329094288;    double sJsgBKKQVnLBFXUHltiH83224817 = -436955682;    double sJsgBKKQVnLBFXUHltiH6690581 = -867543709;    double sJsgBKKQVnLBFXUHltiH54388580 = 5909291;    double sJsgBKKQVnLBFXUHltiH78028449 = -565745230;    double sJsgBKKQVnLBFXUHltiH86257168 = -128909633;    double sJsgBKKQVnLBFXUHltiH82449543 = 32038653;    double sJsgBKKQVnLBFXUHltiH75059531 = 88355623;    double sJsgBKKQVnLBFXUHltiH97229366 = -497151252;    double sJsgBKKQVnLBFXUHltiH99324874 = -469802311;    double sJsgBKKQVnLBFXUHltiH18917715 = -899968024;    double sJsgBKKQVnLBFXUHltiH31997077 = 17744064;    double sJsgBKKQVnLBFXUHltiH28448550 = -328836782;    double sJsgBKKQVnLBFXUHltiH60489169 = -891693701;    double sJsgBKKQVnLBFXUHltiH27568525 = -807621963;    double sJsgBKKQVnLBFXUHltiH94442260 = 12566391;    double sJsgBKKQVnLBFXUHltiH23350631 = -748404967;    double sJsgBKKQVnLBFXUHltiH91269701 = -1662128;    double sJsgBKKQVnLBFXUHltiH43429399 = -483090983;    double sJsgBKKQVnLBFXUHltiH26477994 = -692043708;    double sJsgBKKQVnLBFXUHltiH39355692 = -168526073;    double sJsgBKKQVnLBFXUHltiH82646151 = -953485847;    double sJsgBKKQVnLBFXUHltiH96011849 = -430751748;    double sJsgBKKQVnLBFXUHltiH33096340 = -598461774;    double sJsgBKKQVnLBFXUHltiH64013651 = -129263747;    double sJsgBKKQVnLBFXUHltiH35856801 = -493300320;    double sJsgBKKQVnLBFXUHltiH68899736 = -537749752;    double sJsgBKKQVnLBFXUHltiH30390133 = -843272991;    double sJsgBKKQVnLBFXUHltiH78617531 = -593159047;    double sJsgBKKQVnLBFXUHltiH33463449 = -817044013;    double sJsgBKKQVnLBFXUHltiH48411763 = -107562973;    double sJsgBKKQVnLBFXUHltiH86899675 = -662210107;    double sJsgBKKQVnLBFXUHltiH9503757 = -512888942;    double sJsgBKKQVnLBFXUHltiH68460332 = -735114445;    double sJsgBKKQVnLBFXUHltiH28936856 = -785087790;    double sJsgBKKQVnLBFXUHltiH17811826 = -191534223;    double sJsgBKKQVnLBFXUHltiH60986447 = -854090927;    double sJsgBKKQVnLBFXUHltiH85851405 = -584850811;    double sJsgBKKQVnLBFXUHltiH59596720 = -271730643;    double sJsgBKKQVnLBFXUHltiH11900645 = -586574282;    double sJsgBKKQVnLBFXUHltiH80952521 = -679950715;    double sJsgBKKQVnLBFXUHltiH27333527 = -367631114;    double sJsgBKKQVnLBFXUHltiH57851949 = -201936379;    double sJsgBKKQVnLBFXUHltiH1321069 = -339575543;    double sJsgBKKQVnLBFXUHltiH20516938 = -54494763;    double sJsgBKKQVnLBFXUHltiH83484748 = -581156631;    double sJsgBKKQVnLBFXUHltiH11174066 = -709848952;    double sJsgBKKQVnLBFXUHltiH43593794 = -58065068;    double sJsgBKKQVnLBFXUHltiH1202999 = 99297748;    double sJsgBKKQVnLBFXUHltiH72933520 = -972591053;    double sJsgBKKQVnLBFXUHltiH16689259 = -933770818;    double sJsgBKKQVnLBFXUHltiH25494677 = -664828085;     sJsgBKKQVnLBFXUHltiH21052843 = sJsgBKKQVnLBFXUHltiH70732705;     sJsgBKKQVnLBFXUHltiH70732705 = sJsgBKKQVnLBFXUHltiH64501016;     sJsgBKKQVnLBFXUHltiH64501016 = sJsgBKKQVnLBFXUHltiH79716963;     sJsgBKKQVnLBFXUHltiH79716963 = sJsgBKKQVnLBFXUHltiH59217823;     sJsgBKKQVnLBFXUHltiH59217823 = sJsgBKKQVnLBFXUHltiH96486529;     sJsgBKKQVnLBFXUHltiH96486529 = sJsgBKKQVnLBFXUHltiH5960196;     sJsgBKKQVnLBFXUHltiH5960196 = sJsgBKKQVnLBFXUHltiH28605168;     sJsgBKKQVnLBFXUHltiH28605168 = sJsgBKKQVnLBFXUHltiH62119287;     sJsgBKKQVnLBFXUHltiH62119287 = sJsgBKKQVnLBFXUHltiH63590616;     sJsgBKKQVnLBFXUHltiH63590616 = sJsgBKKQVnLBFXUHltiH28894871;     sJsgBKKQVnLBFXUHltiH28894871 = sJsgBKKQVnLBFXUHltiH41587212;     sJsgBKKQVnLBFXUHltiH41587212 = sJsgBKKQVnLBFXUHltiH89880058;     sJsgBKKQVnLBFXUHltiH89880058 = sJsgBKKQVnLBFXUHltiH41748750;     sJsgBKKQVnLBFXUHltiH41748750 = sJsgBKKQVnLBFXUHltiH37727467;     sJsgBKKQVnLBFXUHltiH37727467 = sJsgBKKQVnLBFXUHltiH96657793;     sJsgBKKQVnLBFXUHltiH96657793 = sJsgBKKQVnLBFXUHltiH72530992;     sJsgBKKQVnLBFXUHltiH72530992 = sJsgBKKQVnLBFXUHltiH79495476;     sJsgBKKQVnLBFXUHltiH79495476 = sJsgBKKQVnLBFXUHltiH75479416;     sJsgBKKQVnLBFXUHltiH75479416 = sJsgBKKQVnLBFXUHltiH72982699;     sJsgBKKQVnLBFXUHltiH72982699 = sJsgBKKQVnLBFXUHltiH59161655;     sJsgBKKQVnLBFXUHltiH59161655 = sJsgBKKQVnLBFXUHltiH45993311;     sJsgBKKQVnLBFXUHltiH45993311 = sJsgBKKQVnLBFXUHltiH73503338;     sJsgBKKQVnLBFXUHltiH73503338 = sJsgBKKQVnLBFXUHltiH65176141;     sJsgBKKQVnLBFXUHltiH65176141 = sJsgBKKQVnLBFXUHltiH60799248;     sJsgBKKQVnLBFXUHltiH60799248 = sJsgBKKQVnLBFXUHltiH27220746;     sJsgBKKQVnLBFXUHltiH27220746 = sJsgBKKQVnLBFXUHltiH68037979;     sJsgBKKQVnLBFXUHltiH68037979 = sJsgBKKQVnLBFXUHltiH45471026;     sJsgBKKQVnLBFXUHltiH45471026 = sJsgBKKQVnLBFXUHltiH1036644;     sJsgBKKQVnLBFXUHltiH1036644 = sJsgBKKQVnLBFXUHltiH67677027;     sJsgBKKQVnLBFXUHltiH67677027 = sJsgBKKQVnLBFXUHltiH40239985;     sJsgBKKQVnLBFXUHltiH40239985 = sJsgBKKQVnLBFXUHltiH37625170;     sJsgBKKQVnLBFXUHltiH37625170 = sJsgBKKQVnLBFXUHltiH98157813;     sJsgBKKQVnLBFXUHltiH98157813 = sJsgBKKQVnLBFXUHltiH63402064;     sJsgBKKQVnLBFXUHltiH63402064 = sJsgBKKQVnLBFXUHltiH2393058;     sJsgBKKQVnLBFXUHltiH2393058 = sJsgBKKQVnLBFXUHltiH55081316;     sJsgBKKQVnLBFXUHltiH55081316 = sJsgBKKQVnLBFXUHltiH645944;     sJsgBKKQVnLBFXUHltiH645944 = sJsgBKKQVnLBFXUHltiH39434652;     sJsgBKKQVnLBFXUHltiH39434652 = sJsgBKKQVnLBFXUHltiH15481826;     sJsgBKKQVnLBFXUHltiH15481826 = sJsgBKKQVnLBFXUHltiH39622616;     sJsgBKKQVnLBFXUHltiH39622616 = sJsgBKKQVnLBFXUHltiH4082963;     sJsgBKKQVnLBFXUHltiH4082963 = sJsgBKKQVnLBFXUHltiH28771523;     sJsgBKKQVnLBFXUHltiH28771523 = sJsgBKKQVnLBFXUHltiH67375780;     sJsgBKKQVnLBFXUHltiH67375780 = sJsgBKKQVnLBFXUHltiH40039890;     sJsgBKKQVnLBFXUHltiH40039890 = sJsgBKKQVnLBFXUHltiH16764379;     sJsgBKKQVnLBFXUHltiH16764379 = sJsgBKKQVnLBFXUHltiH73899573;     sJsgBKKQVnLBFXUHltiH73899573 = sJsgBKKQVnLBFXUHltiH17716990;     sJsgBKKQVnLBFXUHltiH17716990 = sJsgBKKQVnLBFXUHltiH99577647;     sJsgBKKQVnLBFXUHltiH99577647 = sJsgBKKQVnLBFXUHltiH16534170;     sJsgBKKQVnLBFXUHltiH16534170 = sJsgBKKQVnLBFXUHltiH83224817;     sJsgBKKQVnLBFXUHltiH83224817 = sJsgBKKQVnLBFXUHltiH6690581;     sJsgBKKQVnLBFXUHltiH6690581 = sJsgBKKQVnLBFXUHltiH54388580;     sJsgBKKQVnLBFXUHltiH54388580 = sJsgBKKQVnLBFXUHltiH78028449;     sJsgBKKQVnLBFXUHltiH78028449 = sJsgBKKQVnLBFXUHltiH86257168;     sJsgBKKQVnLBFXUHltiH86257168 = sJsgBKKQVnLBFXUHltiH82449543;     sJsgBKKQVnLBFXUHltiH82449543 = sJsgBKKQVnLBFXUHltiH75059531;     sJsgBKKQVnLBFXUHltiH75059531 = sJsgBKKQVnLBFXUHltiH97229366;     sJsgBKKQVnLBFXUHltiH97229366 = sJsgBKKQVnLBFXUHltiH99324874;     sJsgBKKQVnLBFXUHltiH99324874 = sJsgBKKQVnLBFXUHltiH18917715;     sJsgBKKQVnLBFXUHltiH18917715 = sJsgBKKQVnLBFXUHltiH31997077;     sJsgBKKQVnLBFXUHltiH31997077 = sJsgBKKQVnLBFXUHltiH28448550;     sJsgBKKQVnLBFXUHltiH28448550 = sJsgBKKQVnLBFXUHltiH60489169;     sJsgBKKQVnLBFXUHltiH60489169 = sJsgBKKQVnLBFXUHltiH27568525;     sJsgBKKQVnLBFXUHltiH27568525 = sJsgBKKQVnLBFXUHltiH94442260;     sJsgBKKQVnLBFXUHltiH94442260 = sJsgBKKQVnLBFXUHltiH23350631;     sJsgBKKQVnLBFXUHltiH23350631 = sJsgBKKQVnLBFXUHltiH91269701;     sJsgBKKQVnLBFXUHltiH91269701 = sJsgBKKQVnLBFXUHltiH43429399;     sJsgBKKQVnLBFXUHltiH43429399 = sJsgBKKQVnLBFXUHltiH26477994;     sJsgBKKQVnLBFXUHltiH26477994 = sJsgBKKQVnLBFXUHltiH39355692;     sJsgBKKQVnLBFXUHltiH39355692 = sJsgBKKQVnLBFXUHltiH82646151;     sJsgBKKQVnLBFXUHltiH82646151 = sJsgBKKQVnLBFXUHltiH96011849;     sJsgBKKQVnLBFXUHltiH96011849 = sJsgBKKQVnLBFXUHltiH33096340;     sJsgBKKQVnLBFXUHltiH33096340 = sJsgBKKQVnLBFXUHltiH64013651;     sJsgBKKQVnLBFXUHltiH64013651 = sJsgBKKQVnLBFXUHltiH35856801;     sJsgBKKQVnLBFXUHltiH35856801 = sJsgBKKQVnLBFXUHltiH68899736;     sJsgBKKQVnLBFXUHltiH68899736 = sJsgBKKQVnLBFXUHltiH30390133;     sJsgBKKQVnLBFXUHltiH30390133 = sJsgBKKQVnLBFXUHltiH78617531;     sJsgBKKQVnLBFXUHltiH78617531 = sJsgBKKQVnLBFXUHltiH33463449;     sJsgBKKQVnLBFXUHltiH33463449 = sJsgBKKQVnLBFXUHltiH48411763;     sJsgBKKQVnLBFXUHltiH48411763 = sJsgBKKQVnLBFXUHltiH86899675;     sJsgBKKQVnLBFXUHltiH86899675 = sJsgBKKQVnLBFXUHltiH9503757;     sJsgBKKQVnLBFXUHltiH9503757 = sJsgBKKQVnLBFXUHltiH68460332;     sJsgBKKQVnLBFXUHltiH68460332 = sJsgBKKQVnLBFXUHltiH28936856;     sJsgBKKQVnLBFXUHltiH28936856 = sJsgBKKQVnLBFXUHltiH17811826;     sJsgBKKQVnLBFXUHltiH17811826 = sJsgBKKQVnLBFXUHltiH60986447;     sJsgBKKQVnLBFXUHltiH60986447 = sJsgBKKQVnLBFXUHltiH85851405;     sJsgBKKQVnLBFXUHltiH85851405 = sJsgBKKQVnLBFXUHltiH59596720;     sJsgBKKQVnLBFXUHltiH59596720 = sJsgBKKQVnLBFXUHltiH11900645;     sJsgBKKQVnLBFXUHltiH11900645 = sJsgBKKQVnLBFXUHltiH80952521;     sJsgBKKQVnLBFXUHltiH80952521 = sJsgBKKQVnLBFXUHltiH27333527;     sJsgBKKQVnLBFXUHltiH27333527 = sJsgBKKQVnLBFXUHltiH57851949;     sJsgBKKQVnLBFXUHltiH57851949 = sJsgBKKQVnLBFXUHltiH1321069;     sJsgBKKQVnLBFXUHltiH1321069 = sJsgBKKQVnLBFXUHltiH20516938;     sJsgBKKQVnLBFXUHltiH20516938 = sJsgBKKQVnLBFXUHltiH83484748;     sJsgBKKQVnLBFXUHltiH83484748 = sJsgBKKQVnLBFXUHltiH11174066;     sJsgBKKQVnLBFXUHltiH11174066 = sJsgBKKQVnLBFXUHltiH43593794;     sJsgBKKQVnLBFXUHltiH43593794 = sJsgBKKQVnLBFXUHltiH1202999;     sJsgBKKQVnLBFXUHltiH1202999 = sJsgBKKQVnLBFXUHltiH72933520;     sJsgBKKQVnLBFXUHltiH72933520 = sJsgBKKQVnLBFXUHltiH16689259;     sJsgBKKQVnLBFXUHltiH16689259 = sJsgBKKQVnLBFXUHltiH25494677;     sJsgBKKQVnLBFXUHltiH25494677 = sJsgBKKQVnLBFXUHltiH21052843;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fvqhONDsOZGcFdiGzNTO69205455() {     double qJokDEtEuqgLSnjNWXOb46638120 = -89861186;    double qJokDEtEuqgLSnjNWXOb67129848 = -251848825;    double qJokDEtEuqgLSnjNWXOb97380191 = -140020029;    double qJokDEtEuqgLSnjNWXOb25438893 = -634554689;    double qJokDEtEuqgLSnjNWXOb28559828 = -87579637;    double qJokDEtEuqgLSnjNWXOb13495806 = -345444357;    double qJokDEtEuqgLSnjNWXOb11281040 = -588146985;    double qJokDEtEuqgLSnjNWXOb43615293 = -916793746;    double qJokDEtEuqgLSnjNWXOb87448579 = -205292289;    double qJokDEtEuqgLSnjNWXOb82748998 = -761908009;    double qJokDEtEuqgLSnjNWXOb43228425 = -249565887;    double qJokDEtEuqgLSnjNWXOb76904868 = -742283804;    double qJokDEtEuqgLSnjNWXOb66460836 = -204204135;    double qJokDEtEuqgLSnjNWXOb33308467 = -622325857;    double qJokDEtEuqgLSnjNWXOb93996355 = -641119435;    double qJokDEtEuqgLSnjNWXOb4378836 = -776321079;    double qJokDEtEuqgLSnjNWXOb72329108 = -341244354;    double qJokDEtEuqgLSnjNWXOb611381 = -308953675;    double qJokDEtEuqgLSnjNWXOb36944138 = -314959911;    double qJokDEtEuqgLSnjNWXOb34144166 = -561947895;    double qJokDEtEuqgLSnjNWXOb60170921 = -13296986;    double qJokDEtEuqgLSnjNWXOb47496418 = 27083362;    double qJokDEtEuqgLSnjNWXOb77267136 = -543311902;    double qJokDEtEuqgLSnjNWXOb66890184 = -726605162;    double qJokDEtEuqgLSnjNWXOb15702388 = -294759915;    double qJokDEtEuqgLSnjNWXOb56709883 = -622851956;    double qJokDEtEuqgLSnjNWXOb50425888 = -427439707;    double qJokDEtEuqgLSnjNWXOb9369827 = -343844310;    double qJokDEtEuqgLSnjNWXOb8913856 = -964783755;    double qJokDEtEuqgLSnjNWXOb20999518 = -630544198;    double qJokDEtEuqgLSnjNWXOb62286620 = -457588550;    double qJokDEtEuqgLSnjNWXOb74815102 = -111271706;    double qJokDEtEuqgLSnjNWXOb19344866 = -163305052;    double qJokDEtEuqgLSnjNWXOb16989136 = -804057033;    double qJokDEtEuqgLSnjNWXOb3798047 = 19288598;    double qJokDEtEuqgLSnjNWXOb59873166 = -216682346;    double qJokDEtEuqgLSnjNWXOb50291329 = -289889798;    double qJokDEtEuqgLSnjNWXOb15007339 = -903177612;    double qJokDEtEuqgLSnjNWXOb56405404 = -21750889;    double qJokDEtEuqgLSnjNWXOb48406289 = -508656053;    double qJokDEtEuqgLSnjNWXOb51234244 = -524818958;    double qJokDEtEuqgLSnjNWXOb72781112 = -217362719;    double qJokDEtEuqgLSnjNWXOb16576000 = 11368957;    double qJokDEtEuqgLSnjNWXOb10833169 = -50591383;    double qJokDEtEuqgLSnjNWXOb8745255 = -599214327;    double qJokDEtEuqgLSnjNWXOb40299895 = -487601203;    double qJokDEtEuqgLSnjNWXOb17485444 = -477981333;    double qJokDEtEuqgLSnjNWXOb7690418 = -219694974;    double qJokDEtEuqgLSnjNWXOb67345490 = -428770515;    double qJokDEtEuqgLSnjNWXOb37034813 = -284978835;    double qJokDEtEuqgLSnjNWXOb16826828 = -16202311;    double qJokDEtEuqgLSnjNWXOb81647160 = -172169888;    double qJokDEtEuqgLSnjNWXOb96102053 = -33643779;    double qJokDEtEuqgLSnjNWXOb55156077 = -443973415;    double qJokDEtEuqgLSnjNWXOb9034535 = -443051764;    double qJokDEtEuqgLSnjNWXOb99141701 = -16944548;    double qJokDEtEuqgLSnjNWXOb89862712 = -708536924;    double qJokDEtEuqgLSnjNWXOb30490008 = -413414867;    double qJokDEtEuqgLSnjNWXOb9736506 = -239794775;    double qJokDEtEuqgLSnjNWXOb71849945 = -464727682;    double qJokDEtEuqgLSnjNWXOb63069918 = -918004651;    double qJokDEtEuqgLSnjNWXOb1911214 = -144302675;    double qJokDEtEuqgLSnjNWXOb34701437 = -952009992;    double qJokDEtEuqgLSnjNWXOb66449062 = -574748092;    double qJokDEtEuqgLSnjNWXOb20462379 = -204319459;    double qJokDEtEuqgLSnjNWXOb68413323 = -38294181;    double qJokDEtEuqgLSnjNWXOb57560003 = -478978753;    double qJokDEtEuqgLSnjNWXOb49471701 = -400147102;    double qJokDEtEuqgLSnjNWXOb29510421 = -541614455;    double qJokDEtEuqgLSnjNWXOb34123189 = -324437089;    double qJokDEtEuqgLSnjNWXOb54087507 = -386431281;    double qJokDEtEuqgLSnjNWXOb57321769 = -438066743;    double qJokDEtEuqgLSnjNWXOb44205977 = -187202786;    double qJokDEtEuqgLSnjNWXOb88537848 = -806303858;    double qJokDEtEuqgLSnjNWXOb82909921 = 62871062;    double qJokDEtEuqgLSnjNWXOb87389808 = -795934268;    double qJokDEtEuqgLSnjNWXOb30920419 = -984285595;    double qJokDEtEuqgLSnjNWXOb66433967 = -392720519;    double qJokDEtEuqgLSnjNWXOb58144929 = -27390835;    double qJokDEtEuqgLSnjNWXOb75402492 = -807158712;    double qJokDEtEuqgLSnjNWXOb39224439 = -44870623;    double qJokDEtEuqgLSnjNWXOb42735470 = -107744733;    double qJokDEtEuqgLSnjNWXOb42024336 = -915073796;    double qJokDEtEuqgLSnjNWXOb71879043 = -579804920;    double qJokDEtEuqgLSnjNWXOb4172691 = -514341887;    double qJokDEtEuqgLSnjNWXOb80639460 = -185418662;    double qJokDEtEuqgLSnjNWXOb78713049 = 22372072;    double qJokDEtEuqgLSnjNWXOb64188788 = -719331638;    double qJokDEtEuqgLSnjNWXOb7954601 = -261005269;    double qJokDEtEuqgLSnjNWXOb4656345 = -963766854;    double qJokDEtEuqgLSnjNWXOb70010454 = -508145423;    double qJokDEtEuqgLSnjNWXOb19801322 = -876474931;    double qJokDEtEuqgLSnjNWXOb5270834 = -563382837;    double qJokDEtEuqgLSnjNWXOb84555459 = -557023208;    double qJokDEtEuqgLSnjNWXOb85336371 = -590651403;    double qJokDEtEuqgLSnjNWXOb49323031 = -280516283;    double qJokDEtEuqgLSnjNWXOb38079676 = -265352727;    double qJokDEtEuqgLSnjNWXOb50126937 = -413882952;    double qJokDEtEuqgLSnjNWXOb90370790 = -846271924;    double qJokDEtEuqgLSnjNWXOb40331932 = -89861186;     qJokDEtEuqgLSnjNWXOb46638120 = qJokDEtEuqgLSnjNWXOb67129848;     qJokDEtEuqgLSnjNWXOb67129848 = qJokDEtEuqgLSnjNWXOb97380191;     qJokDEtEuqgLSnjNWXOb97380191 = qJokDEtEuqgLSnjNWXOb25438893;     qJokDEtEuqgLSnjNWXOb25438893 = qJokDEtEuqgLSnjNWXOb28559828;     qJokDEtEuqgLSnjNWXOb28559828 = qJokDEtEuqgLSnjNWXOb13495806;     qJokDEtEuqgLSnjNWXOb13495806 = qJokDEtEuqgLSnjNWXOb11281040;     qJokDEtEuqgLSnjNWXOb11281040 = qJokDEtEuqgLSnjNWXOb43615293;     qJokDEtEuqgLSnjNWXOb43615293 = qJokDEtEuqgLSnjNWXOb87448579;     qJokDEtEuqgLSnjNWXOb87448579 = qJokDEtEuqgLSnjNWXOb82748998;     qJokDEtEuqgLSnjNWXOb82748998 = qJokDEtEuqgLSnjNWXOb43228425;     qJokDEtEuqgLSnjNWXOb43228425 = qJokDEtEuqgLSnjNWXOb76904868;     qJokDEtEuqgLSnjNWXOb76904868 = qJokDEtEuqgLSnjNWXOb66460836;     qJokDEtEuqgLSnjNWXOb66460836 = qJokDEtEuqgLSnjNWXOb33308467;     qJokDEtEuqgLSnjNWXOb33308467 = qJokDEtEuqgLSnjNWXOb93996355;     qJokDEtEuqgLSnjNWXOb93996355 = qJokDEtEuqgLSnjNWXOb4378836;     qJokDEtEuqgLSnjNWXOb4378836 = qJokDEtEuqgLSnjNWXOb72329108;     qJokDEtEuqgLSnjNWXOb72329108 = qJokDEtEuqgLSnjNWXOb611381;     qJokDEtEuqgLSnjNWXOb611381 = qJokDEtEuqgLSnjNWXOb36944138;     qJokDEtEuqgLSnjNWXOb36944138 = qJokDEtEuqgLSnjNWXOb34144166;     qJokDEtEuqgLSnjNWXOb34144166 = qJokDEtEuqgLSnjNWXOb60170921;     qJokDEtEuqgLSnjNWXOb60170921 = qJokDEtEuqgLSnjNWXOb47496418;     qJokDEtEuqgLSnjNWXOb47496418 = qJokDEtEuqgLSnjNWXOb77267136;     qJokDEtEuqgLSnjNWXOb77267136 = qJokDEtEuqgLSnjNWXOb66890184;     qJokDEtEuqgLSnjNWXOb66890184 = qJokDEtEuqgLSnjNWXOb15702388;     qJokDEtEuqgLSnjNWXOb15702388 = qJokDEtEuqgLSnjNWXOb56709883;     qJokDEtEuqgLSnjNWXOb56709883 = qJokDEtEuqgLSnjNWXOb50425888;     qJokDEtEuqgLSnjNWXOb50425888 = qJokDEtEuqgLSnjNWXOb9369827;     qJokDEtEuqgLSnjNWXOb9369827 = qJokDEtEuqgLSnjNWXOb8913856;     qJokDEtEuqgLSnjNWXOb8913856 = qJokDEtEuqgLSnjNWXOb20999518;     qJokDEtEuqgLSnjNWXOb20999518 = qJokDEtEuqgLSnjNWXOb62286620;     qJokDEtEuqgLSnjNWXOb62286620 = qJokDEtEuqgLSnjNWXOb74815102;     qJokDEtEuqgLSnjNWXOb74815102 = qJokDEtEuqgLSnjNWXOb19344866;     qJokDEtEuqgLSnjNWXOb19344866 = qJokDEtEuqgLSnjNWXOb16989136;     qJokDEtEuqgLSnjNWXOb16989136 = qJokDEtEuqgLSnjNWXOb3798047;     qJokDEtEuqgLSnjNWXOb3798047 = qJokDEtEuqgLSnjNWXOb59873166;     qJokDEtEuqgLSnjNWXOb59873166 = qJokDEtEuqgLSnjNWXOb50291329;     qJokDEtEuqgLSnjNWXOb50291329 = qJokDEtEuqgLSnjNWXOb15007339;     qJokDEtEuqgLSnjNWXOb15007339 = qJokDEtEuqgLSnjNWXOb56405404;     qJokDEtEuqgLSnjNWXOb56405404 = qJokDEtEuqgLSnjNWXOb48406289;     qJokDEtEuqgLSnjNWXOb48406289 = qJokDEtEuqgLSnjNWXOb51234244;     qJokDEtEuqgLSnjNWXOb51234244 = qJokDEtEuqgLSnjNWXOb72781112;     qJokDEtEuqgLSnjNWXOb72781112 = qJokDEtEuqgLSnjNWXOb16576000;     qJokDEtEuqgLSnjNWXOb16576000 = qJokDEtEuqgLSnjNWXOb10833169;     qJokDEtEuqgLSnjNWXOb10833169 = qJokDEtEuqgLSnjNWXOb8745255;     qJokDEtEuqgLSnjNWXOb8745255 = qJokDEtEuqgLSnjNWXOb40299895;     qJokDEtEuqgLSnjNWXOb40299895 = qJokDEtEuqgLSnjNWXOb17485444;     qJokDEtEuqgLSnjNWXOb17485444 = qJokDEtEuqgLSnjNWXOb7690418;     qJokDEtEuqgLSnjNWXOb7690418 = qJokDEtEuqgLSnjNWXOb67345490;     qJokDEtEuqgLSnjNWXOb67345490 = qJokDEtEuqgLSnjNWXOb37034813;     qJokDEtEuqgLSnjNWXOb37034813 = qJokDEtEuqgLSnjNWXOb16826828;     qJokDEtEuqgLSnjNWXOb16826828 = qJokDEtEuqgLSnjNWXOb81647160;     qJokDEtEuqgLSnjNWXOb81647160 = qJokDEtEuqgLSnjNWXOb96102053;     qJokDEtEuqgLSnjNWXOb96102053 = qJokDEtEuqgLSnjNWXOb55156077;     qJokDEtEuqgLSnjNWXOb55156077 = qJokDEtEuqgLSnjNWXOb9034535;     qJokDEtEuqgLSnjNWXOb9034535 = qJokDEtEuqgLSnjNWXOb99141701;     qJokDEtEuqgLSnjNWXOb99141701 = qJokDEtEuqgLSnjNWXOb89862712;     qJokDEtEuqgLSnjNWXOb89862712 = qJokDEtEuqgLSnjNWXOb30490008;     qJokDEtEuqgLSnjNWXOb30490008 = qJokDEtEuqgLSnjNWXOb9736506;     qJokDEtEuqgLSnjNWXOb9736506 = qJokDEtEuqgLSnjNWXOb71849945;     qJokDEtEuqgLSnjNWXOb71849945 = qJokDEtEuqgLSnjNWXOb63069918;     qJokDEtEuqgLSnjNWXOb63069918 = qJokDEtEuqgLSnjNWXOb1911214;     qJokDEtEuqgLSnjNWXOb1911214 = qJokDEtEuqgLSnjNWXOb34701437;     qJokDEtEuqgLSnjNWXOb34701437 = qJokDEtEuqgLSnjNWXOb66449062;     qJokDEtEuqgLSnjNWXOb66449062 = qJokDEtEuqgLSnjNWXOb20462379;     qJokDEtEuqgLSnjNWXOb20462379 = qJokDEtEuqgLSnjNWXOb68413323;     qJokDEtEuqgLSnjNWXOb68413323 = qJokDEtEuqgLSnjNWXOb57560003;     qJokDEtEuqgLSnjNWXOb57560003 = qJokDEtEuqgLSnjNWXOb49471701;     qJokDEtEuqgLSnjNWXOb49471701 = qJokDEtEuqgLSnjNWXOb29510421;     qJokDEtEuqgLSnjNWXOb29510421 = qJokDEtEuqgLSnjNWXOb34123189;     qJokDEtEuqgLSnjNWXOb34123189 = qJokDEtEuqgLSnjNWXOb54087507;     qJokDEtEuqgLSnjNWXOb54087507 = qJokDEtEuqgLSnjNWXOb57321769;     qJokDEtEuqgLSnjNWXOb57321769 = qJokDEtEuqgLSnjNWXOb44205977;     qJokDEtEuqgLSnjNWXOb44205977 = qJokDEtEuqgLSnjNWXOb88537848;     qJokDEtEuqgLSnjNWXOb88537848 = qJokDEtEuqgLSnjNWXOb82909921;     qJokDEtEuqgLSnjNWXOb82909921 = qJokDEtEuqgLSnjNWXOb87389808;     qJokDEtEuqgLSnjNWXOb87389808 = qJokDEtEuqgLSnjNWXOb30920419;     qJokDEtEuqgLSnjNWXOb30920419 = qJokDEtEuqgLSnjNWXOb66433967;     qJokDEtEuqgLSnjNWXOb66433967 = qJokDEtEuqgLSnjNWXOb58144929;     qJokDEtEuqgLSnjNWXOb58144929 = qJokDEtEuqgLSnjNWXOb75402492;     qJokDEtEuqgLSnjNWXOb75402492 = qJokDEtEuqgLSnjNWXOb39224439;     qJokDEtEuqgLSnjNWXOb39224439 = qJokDEtEuqgLSnjNWXOb42735470;     qJokDEtEuqgLSnjNWXOb42735470 = qJokDEtEuqgLSnjNWXOb42024336;     qJokDEtEuqgLSnjNWXOb42024336 = qJokDEtEuqgLSnjNWXOb71879043;     qJokDEtEuqgLSnjNWXOb71879043 = qJokDEtEuqgLSnjNWXOb4172691;     qJokDEtEuqgLSnjNWXOb4172691 = qJokDEtEuqgLSnjNWXOb80639460;     qJokDEtEuqgLSnjNWXOb80639460 = qJokDEtEuqgLSnjNWXOb78713049;     qJokDEtEuqgLSnjNWXOb78713049 = qJokDEtEuqgLSnjNWXOb64188788;     qJokDEtEuqgLSnjNWXOb64188788 = qJokDEtEuqgLSnjNWXOb7954601;     qJokDEtEuqgLSnjNWXOb7954601 = qJokDEtEuqgLSnjNWXOb4656345;     qJokDEtEuqgLSnjNWXOb4656345 = qJokDEtEuqgLSnjNWXOb70010454;     qJokDEtEuqgLSnjNWXOb70010454 = qJokDEtEuqgLSnjNWXOb19801322;     qJokDEtEuqgLSnjNWXOb19801322 = qJokDEtEuqgLSnjNWXOb5270834;     qJokDEtEuqgLSnjNWXOb5270834 = qJokDEtEuqgLSnjNWXOb84555459;     qJokDEtEuqgLSnjNWXOb84555459 = qJokDEtEuqgLSnjNWXOb85336371;     qJokDEtEuqgLSnjNWXOb85336371 = qJokDEtEuqgLSnjNWXOb49323031;     qJokDEtEuqgLSnjNWXOb49323031 = qJokDEtEuqgLSnjNWXOb38079676;     qJokDEtEuqgLSnjNWXOb38079676 = qJokDEtEuqgLSnjNWXOb50126937;     qJokDEtEuqgLSnjNWXOb50126937 = qJokDEtEuqgLSnjNWXOb90370790;     qJokDEtEuqgLSnjNWXOb90370790 = qJokDEtEuqgLSnjNWXOb40331932;     qJokDEtEuqgLSnjNWXOb40331932 = qJokDEtEuqgLSnjNWXOb46638120;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XpwuXmkrVoWxShBMjYhW96799712() {     double LroiMdhoLqPUrrOmcGEM72223397 = -614894286;    double LroiMdhoLqPUrrOmcGEM63526991 = -507326602;    double LroiMdhoLqPUrrOmcGEM30259367 = 57518435;    double LroiMdhoLqPUrrOmcGEM71160822 = -641877582;    double LroiMdhoLqPUrrOmcGEM97901832 = -134501924;    double LroiMdhoLqPUrrOmcGEM30505083 = -500425361;    double LroiMdhoLqPUrrOmcGEM16601885 = -70418191;    double LroiMdhoLqPUrrOmcGEM58625417 = -197475625;    double LroiMdhoLqPUrrOmcGEM12777873 = -701516334;    double LroiMdhoLqPUrrOmcGEM1907381 = 3530468;    double LroiMdhoLqPUrrOmcGEM57561979 = -559993773;    double LroiMdhoLqPUrrOmcGEM12222525 = -85992711;    double LroiMdhoLqPUrrOmcGEM43041614 = 31547500;    double LroiMdhoLqPUrrOmcGEM24868184 = -596850150;    double LroiMdhoLqPUrrOmcGEM50265243 = -529665392;    double LroiMdhoLqPUrrOmcGEM12099879 = -112512556;    double LroiMdhoLqPUrrOmcGEM72127224 = -29564148;    double LroiMdhoLqPUrrOmcGEM21727286 = -825231035;    double LroiMdhoLqPUrrOmcGEM98408858 = 2066232;    double LroiMdhoLqPUrrOmcGEM95305632 = -536387271;    double LroiMdhoLqPUrrOmcGEM61180186 = -474996767;    double LroiMdhoLqPUrrOmcGEM48999525 = -392649567;    double LroiMdhoLqPUrrOmcGEM81030933 = -587404007;    double LroiMdhoLqPUrrOmcGEM68604226 = -585454141;    double LroiMdhoLqPUrrOmcGEM70605526 = -962256057;    double LroiMdhoLqPUrrOmcGEM86199019 = -187302497;    double LroiMdhoLqPUrrOmcGEM32813798 = 6747158;    double LroiMdhoLqPUrrOmcGEM73268626 = -573506543;    double LroiMdhoLqPUrrOmcGEM16791069 = -101077605;    double LroiMdhoLqPUrrOmcGEM74322009 = -539453760;    double LroiMdhoLqPUrrOmcGEM84333254 = -236235581;    double LroiMdhoLqPUrrOmcGEM12005035 = -385067540;    double LroiMdhoLqPUrrOmcGEM40531917 = -611126189;    double LroiMdhoLqPUrrOmcGEM70576206 = -860202005;    double LroiMdhoLqPUrrOmcGEM5203035 = -682147313;    double LroiMdhoLqPUrrOmcGEM64665017 = -734277062;    double LroiMdhoLqPUrrOmcGEM99936714 = -770401743;    double LroiMdhoLqPUrrOmcGEM90580025 = -751892437;    double LroiMdhoLqPUrrOmcGEM97328982 = -480089211;    double LroiMdhoLqPUrrOmcGEM57189963 = -978626372;    double LroiMdhoLqPUrrOmcGEM98385524 = 120852;    double LroiMdhoLqPUrrOmcGEM16790703 = -726401223;    double LroiMdhoLqPUrrOmcGEM65776219 = -17237424;    double LroiMdhoLqPUrrOmcGEM81626447 = -519006982;    double LroiMdhoLqPUrrOmcGEM726131 = -538235445;    double LroiMdhoLqPUrrOmcGEM6700218 = 89851259;    double LroiMdhoLqPUrrOmcGEM17253898 = -510450194;    double LroiMdhoLqPUrrOmcGEM15803189 = -412877821;    double LroiMdhoLqPUrrOmcGEM18156810 = -528446741;    double LroiMdhoLqPUrrOmcGEM90844808 = -133001988;    double LroiMdhoLqPUrrOmcGEM26963076 = -264860914;    double LroiMdhoLqPUrrOmcGEM8905740 = -350249068;    double LroiMdhoLqPUrrOmcGEM14175657 = -601542328;    double LroiMdhoLqPUrrOmcGEM24054986 = -759037196;    double LroiMdhoLqPUrrOmcGEM35619526 = -918142182;    double LroiMdhoLqPUrrOmcGEM23223872 = -122244720;    double LroiMdhoLqPUrrOmcGEM82496057 = -919922595;    double LroiMdhoLqPUrrOmcGEM61655141 = -357027424;    double LroiMdhoLqPUrrOmcGEM555296 = -679621526;    double LroiMdhoLqPUrrOmcGEM11702814 = -947199427;    double LroiMdhoLqPUrrOmcGEM97691285 = -407172519;    double LroiMdhoLqPUrrOmcGEM43333258 = -496911649;    double LroiMdhoLqPUrrOmcGEM41834348 = 3601980;    double LroiMdhoLqPUrrOmcGEM38455864 = -62062574;    double LroiMdhoLqPUrrOmcGEM17574127 = -760233952;    double LroiMdhoLqPUrrOmcGEM45556944 = -74926234;    double LroiMdhoLqPUrrOmcGEM71690607 = -474866522;    double LroiMdhoLqPUrrOmcGEM72465407 = -108250496;    double LroiMdhoLqPUrrOmcGEM19665150 = -914702837;    double LroiMdhoLqPUrrOmcGEM85600226 = -795388331;    double LroiMdhoLqPUrrOmcGEM12163165 = -342110813;    double LroiMdhoLqPUrrOmcGEM81547199 = -277671712;    double LroiMdhoLqPUrrOmcGEM24398303 = -245141825;    double LroiMdhoLqPUrrOmcGEM41218896 = -19307397;    double LroiMdhoLqPUrrOmcGEM96920107 = -436508124;    double LroiMdhoLqPUrrOmcGEM44389483 = -748595545;    double LroiMdhoLqPUrrOmcGEM83223306 = -275412143;    double LroiMdhoLqPUrrOmcGEM99404485 = 31602974;    double LroiMdhoLqPUrrOmcGEM67878095 = 52781303;    double LroiMdhoLqPUrrOmcGEM63905309 = -952107317;    double LroiMdhoLqPUrrOmcGEM68945121 = -676852304;    double LroiMdhoLqPUrrOmcGEM17010609 = -580375021;    double LroiMdhoLqPUrrOmcGEM55111817 = 54940198;    double LroiMdhoLqPUrrOmcGEM25946261 = -968075617;    double LroiMdhoLqPUrrOmcGEM47358934 = -174592846;    double LroiMdhoLqPUrrOmcGEM75427514 = -885986514;    double LroiMdhoLqPUrrOmcGEM97829377 = -783525213;    double LroiMdhoLqPUrrOmcGEM16476932 = -852088994;    double LroiMdhoLqPUrrOmcGEM34956681 = -942059824;    double LroiMdhoLqPUrrOmcGEM81979162 = -459902594;    double LroiMdhoLqPUrrOmcGEM82168959 = -814354468;    double LroiMdhoLqPUrrOmcGEM38281574 = -313374320;    double LroiMdhoLqPUrrOmcGEM90024730 = 27729089;    double LroiMdhoLqPUrrOmcGEM85626169 = -532889784;    double LroiMdhoLqPUrrOmcGEM59498678 = -471453853;    double LroiMdhoLqPUrrOmcGEM55052267 = -502967499;    double LroiMdhoLqPUrrOmcGEM74956354 = -630003203;    double LroiMdhoLqPUrrOmcGEM27320355 = -955174850;    double LroiMdhoLqPUrrOmcGEM64052321 = -758773031;    double LroiMdhoLqPUrrOmcGEM55169187 = -614894286;     LroiMdhoLqPUrrOmcGEM72223397 = LroiMdhoLqPUrrOmcGEM63526991;     LroiMdhoLqPUrrOmcGEM63526991 = LroiMdhoLqPUrrOmcGEM30259367;     LroiMdhoLqPUrrOmcGEM30259367 = LroiMdhoLqPUrrOmcGEM71160822;     LroiMdhoLqPUrrOmcGEM71160822 = LroiMdhoLqPUrrOmcGEM97901832;     LroiMdhoLqPUrrOmcGEM97901832 = LroiMdhoLqPUrrOmcGEM30505083;     LroiMdhoLqPUrrOmcGEM30505083 = LroiMdhoLqPUrrOmcGEM16601885;     LroiMdhoLqPUrrOmcGEM16601885 = LroiMdhoLqPUrrOmcGEM58625417;     LroiMdhoLqPUrrOmcGEM58625417 = LroiMdhoLqPUrrOmcGEM12777873;     LroiMdhoLqPUrrOmcGEM12777873 = LroiMdhoLqPUrrOmcGEM1907381;     LroiMdhoLqPUrrOmcGEM1907381 = LroiMdhoLqPUrrOmcGEM57561979;     LroiMdhoLqPUrrOmcGEM57561979 = LroiMdhoLqPUrrOmcGEM12222525;     LroiMdhoLqPUrrOmcGEM12222525 = LroiMdhoLqPUrrOmcGEM43041614;     LroiMdhoLqPUrrOmcGEM43041614 = LroiMdhoLqPUrrOmcGEM24868184;     LroiMdhoLqPUrrOmcGEM24868184 = LroiMdhoLqPUrrOmcGEM50265243;     LroiMdhoLqPUrrOmcGEM50265243 = LroiMdhoLqPUrrOmcGEM12099879;     LroiMdhoLqPUrrOmcGEM12099879 = LroiMdhoLqPUrrOmcGEM72127224;     LroiMdhoLqPUrrOmcGEM72127224 = LroiMdhoLqPUrrOmcGEM21727286;     LroiMdhoLqPUrrOmcGEM21727286 = LroiMdhoLqPUrrOmcGEM98408858;     LroiMdhoLqPUrrOmcGEM98408858 = LroiMdhoLqPUrrOmcGEM95305632;     LroiMdhoLqPUrrOmcGEM95305632 = LroiMdhoLqPUrrOmcGEM61180186;     LroiMdhoLqPUrrOmcGEM61180186 = LroiMdhoLqPUrrOmcGEM48999525;     LroiMdhoLqPUrrOmcGEM48999525 = LroiMdhoLqPUrrOmcGEM81030933;     LroiMdhoLqPUrrOmcGEM81030933 = LroiMdhoLqPUrrOmcGEM68604226;     LroiMdhoLqPUrrOmcGEM68604226 = LroiMdhoLqPUrrOmcGEM70605526;     LroiMdhoLqPUrrOmcGEM70605526 = LroiMdhoLqPUrrOmcGEM86199019;     LroiMdhoLqPUrrOmcGEM86199019 = LroiMdhoLqPUrrOmcGEM32813798;     LroiMdhoLqPUrrOmcGEM32813798 = LroiMdhoLqPUrrOmcGEM73268626;     LroiMdhoLqPUrrOmcGEM73268626 = LroiMdhoLqPUrrOmcGEM16791069;     LroiMdhoLqPUrrOmcGEM16791069 = LroiMdhoLqPUrrOmcGEM74322009;     LroiMdhoLqPUrrOmcGEM74322009 = LroiMdhoLqPUrrOmcGEM84333254;     LroiMdhoLqPUrrOmcGEM84333254 = LroiMdhoLqPUrrOmcGEM12005035;     LroiMdhoLqPUrrOmcGEM12005035 = LroiMdhoLqPUrrOmcGEM40531917;     LroiMdhoLqPUrrOmcGEM40531917 = LroiMdhoLqPUrrOmcGEM70576206;     LroiMdhoLqPUrrOmcGEM70576206 = LroiMdhoLqPUrrOmcGEM5203035;     LroiMdhoLqPUrrOmcGEM5203035 = LroiMdhoLqPUrrOmcGEM64665017;     LroiMdhoLqPUrrOmcGEM64665017 = LroiMdhoLqPUrrOmcGEM99936714;     LroiMdhoLqPUrrOmcGEM99936714 = LroiMdhoLqPUrrOmcGEM90580025;     LroiMdhoLqPUrrOmcGEM90580025 = LroiMdhoLqPUrrOmcGEM97328982;     LroiMdhoLqPUrrOmcGEM97328982 = LroiMdhoLqPUrrOmcGEM57189963;     LroiMdhoLqPUrrOmcGEM57189963 = LroiMdhoLqPUrrOmcGEM98385524;     LroiMdhoLqPUrrOmcGEM98385524 = LroiMdhoLqPUrrOmcGEM16790703;     LroiMdhoLqPUrrOmcGEM16790703 = LroiMdhoLqPUrrOmcGEM65776219;     LroiMdhoLqPUrrOmcGEM65776219 = LroiMdhoLqPUrrOmcGEM81626447;     LroiMdhoLqPUrrOmcGEM81626447 = LroiMdhoLqPUrrOmcGEM726131;     LroiMdhoLqPUrrOmcGEM726131 = LroiMdhoLqPUrrOmcGEM6700218;     LroiMdhoLqPUrrOmcGEM6700218 = LroiMdhoLqPUrrOmcGEM17253898;     LroiMdhoLqPUrrOmcGEM17253898 = LroiMdhoLqPUrrOmcGEM15803189;     LroiMdhoLqPUrrOmcGEM15803189 = LroiMdhoLqPUrrOmcGEM18156810;     LroiMdhoLqPUrrOmcGEM18156810 = LroiMdhoLqPUrrOmcGEM90844808;     LroiMdhoLqPUrrOmcGEM90844808 = LroiMdhoLqPUrrOmcGEM26963076;     LroiMdhoLqPUrrOmcGEM26963076 = LroiMdhoLqPUrrOmcGEM8905740;     LroiMdhoLqPUrrOmcGEM8905740 = LroiMdhoLqPUrrOmcGEM14175657;     LroiMdhoLqPUrrOmcGEM14175657 = LroiMdhoLqPUrrOmcGEM24054986;     LroiMdhoLqPUrrOmcGEM24054986 = LroiMdhoLqPUrrOmcGEM35619526;     LroiMdhoLqPUrrOmcGEM35619526 = LroiMdhoLqPUrrOmcGEM23223872;     LroiMdhoLqPUrrOmcGEM23223872 = LroiMdhoLqPUrrOmcGEM82496057;     LroiMdhoLqPUrrOmcGEM82496057 = LroiMdhoLqPUrrOmcGEM61655141;     LroiMdhoLqPUrrOmcGEM61655141 = LroiMdhoLqPUrrOmcGEM555296;     LroiMdhoLqPUrrOmcGEM555296 = LroiMdhoLqPUrrOmcGEM11702814;     LroiMdhoLqPUrrOmcGEM11702814 = LroiMdhoLqPUrrOmcGEM97691285;     LroiMdhoLqPUrrOmcGEM97691285 = LroiMdhoLqPUrrOmcGEM43333258;     LroiMdhoLqPUrrOmcGEM43333258 = LroiMdhoLqPUrrOmcGEM41834348;     LroiMdhoLqPUrrOmcGEM41834348 = LroiMdhoLqPUrrOmcGEM38455864;     LroiMdhoLqPUrrOmcGEM38455864 = LroiMdhoLqPUrrOmcGEM17574127;     LroiMdhoLqPUrrOmcGEM17574127 = LroiMdhoLqPUrrOmcGEM45556944;     LroiMdhoLqPUrrOmcGEM45556944 = LroiMdhoLqPUrrOmcGEM71690607;     LroiMdhoLqPUrrOmcGEM71690607 = LroiMdhoLqPUrrOmcGEM72465407;     LroiMdhoLqPUrrOmcGEM72465407 = LroiMdhoLqPUrrOmcGEM19665150;     LroiMdhoLqPUrrOmcGEM19665150 = LroiMdhoLqPUrrOmcGEM85600226;     LroiMdhoLqPUrrOmcGEM85600226 = LroiMdhoLqPUrrOmcGEM12163165;     LroiMdhoLqPUrrOmcGEM12163165 = LroiMdhoLqPUrrOmcGEM81547199;     LroiMdhoLqPUrrOmcGEM81547199 = LroiMdhoLqPUrrOmcGEM24398303;     LroiMdhoLqPUrrOmcGEM24398303 = LroiMdhoLqPUrrOmcGEM41218896;     LroiMdhoLqPUrrOmcGEM41218896 = LroiMdhoLqPUrrOmcGEM96920107;     LroiMdhoLqPUrrOmcGEM96920107 = LroiMdhoLqPUrrOmcGEM44389483;     LroiMdhoLqPUrrOmcGEM44389483 = LroiMdhoLqPUrrOmcGEM83223306;     LroiMdhoLqPUrrOmcGEM83223306 = LroiMdhoLqPUrrOmcGEM99404485;     LroiMdhoLqPUrrOmcGEM99404485 = LroiMdhoLqPUrrOmcGEM67878095;     LroiMdhoLqPUrrOmcGEM67878095 = LroiMdhoLqPUrrOmcGEM63905309;     LroiMdhoLqPUrrOmcGEM63905309 = LroiMdhoLqPUrrOmcGEM68945121;     LroiMdhoLqPUrrOmcGEM68945121 = LroiMdhoLqPUrrOmcGEM17010609;     LroiMdhoLqPUrrOmcGEM17010609 = LroiMdhoLqPUrrOmcGEM55111817;     LroiMdhoLqPUrrOmcGEM55111817 = LroiMdhoLqPUrrOmcGEM25946261;     LroiMdhoLqPUrrOmcGEM25946261 = LroiMdhoLqPUrrOmcGEM47358934;     LroiMdhoLqPUrrOmcGEM47358934 = LroiMdhoLqPUrrOmcGEM75427514;     LroiMdhoLqPUrrOmcGEM75427514 = LroiMdhoLqPUrrOmcGEM97829377;     LroiMdhoLqPUrrOmcGEM97829377 = LroiMdhoLqPUrrOmcGEM16476932;     LroiMdhoLqPUrrOmcGEM16476932 = LroiMdhoLqPUrrOmcGEM34956681;     LroiMdhoLqPUrrOmcGEM34956681 = LroiMdhoLqPUrrOmcGEM81979162;     LroiMdhoLqPUrrOmcGEM81979162 = LroiMdhoLqPUrrOmcGEM82168959;     LroiMdhoLqPUrrOmcGEM82168959 = LroiMdhoLqPUrrOmcGEM38281574;     LroiMdhoLqPUrrOmcGEM38281574 = LroiMdhoLqPUrrOmcGEM90024730;     LroiMdhoLqPUrrOmcGEM90024730 = LroiMdhoLqPUrrOmcGEM85626169;     LroiMdhoLqPUrrOmcGEM85626169 = LroiMdhoLqPUrrOmcGEM59498678;     LroiMdhoLqPUrrOmcGEM59498678 = LroiMdhoLqPUrrOmcGEM55052267;     LroiMdhoLqPUrrOmcGEM55052267 = LroiMdhoLqPUrrOmcGEM74956354;     LroiMdhoLqPUrrOmcGEM74956354 = LroiMdhoLqPUrrOmcGEM27320355;     LroiMdhoLqPUrrOmcGEM27320355 = LroiMdhoLqPUrrOmcGEM64052321;     LroiMdhoLqPUrrOmcGEM64052321 = LroiMdhoLqPUrrOmcGEM55169187;     LroiMdhoLqPUrrOmcGEM55169187 = LroiMdhoLqPUrrOmcGEM72223397;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AXNAPAMlKBsnlbEtdegB24393969() {     double HEOjkkZacoEUnvATsuvF97808673 = -39927387;    double HEOjkkZacoEUnvATsuvF59924135 = -762804379;    double HEOjkkZacoEUnvATsuvF63138542 = -844943101;    double HEOjkkZacoEUnvATsuvF16882752 = -649200476;    double HEOjkkZacoEUnvATsuvF67243837 = -181424211;    double HEOjkkZacoEUnvATsuvF47514359 = -655406364;    double HEOjkkZacoEUnvATsuvF21922729 = -652689398;    double HEOjkkZacoEUnvATsuvF73635541 = -578157504;    double HEOjkkZacoEUnvATsuvF38107165 = -97740378;    double HEOjkkZacoEUnvATsuvF21065763 = -331031056;    double HEOjkkZacoEUnvATsuvF71895533 = -870421660;    double HEOjkkZacoEUnvATsuvF47540180 = -529701618;    double HEOjkkZacoEUnvATsuvF19622392 = -832700865;    double HEOjkkZacoEUnvATsuvF16427901 = -571374444;    double HEOjkkZacoEUnvATsuvF6534132 = -418211350;    double HEOjkkZacoEUnvATsuvF19820922 = -548704033;    double HEOjkkZacoEUnvATsuvF71925341 = -817883942;    double HEOjkkZacoEUnvATsuvF42843190 = -241508396;    double HEOjkkZacoEUnvATsuvF59873580 = -780907626;    double HEOjkkZacoEUnvATsuvF56467099 = -510826646;    double HEOjkkZacoEUnvATsuvF62189451 = -936696548;    double HEOjkkZacoEUnvATsuvF50502631 = -812382497;    double HEOjkkZacoEUnvATsuvF84794731 = -631496113;    double HEOjkkZacoEUnvATsuvF70318268 = -444303121;    double HEOjkkZacoEUnvATsuvF25508666 = -529752200;    double HEOjkkZacoEUnvATsuvF15688157 = -851753039;    double HEOjkkZacoEUnvATsuvF15201707 = -659065976;    double HEOjkkZacoEUnvATsuvF37167427 = -803168775;    double HEOjkkZacoEUnvATsuvF24668281 = -337371455;    double HEOjkkZacoEUnvATsuvF27644500 = -448363321;    double HEOjkkZacoEUnvATsuvF6379889 = -14882612;    double HEOjkkZacoEUnvATsuvF49194967 = -658863374;    double HEOjkkZacoEUnvATsuvF61718969 = 41052674;    double HEOjkkZacoEUnvATsuvF24163278 = -916346976;    double HEOjkkZacoEUnvATsuvF6608023 = -283583225;    double HEOjkkZacoEUnvATsuvF69456867 = -151871778;    double HEOjkkZacoEUnvATsuvF49582100 = -150913688;    double HEOjkkZacoEUnvATsuvF66152712 = -600607262;    double HEOjkkZacoEUnvATsuvF38252562 = -938427533;    double HEOjkkZacoEUnvATsuvF65973637 = -348596691;    double HEOjkkZacoEUnvATsuvF45536806 = -574939337;    double HEOjkkZacoEUnvATsuvF60800292 = -135439727;    double HEOjkkZacoEUnvATsuvF14976438 = -45843806;    double HEOjkkZacoEUnvATsuvF52419727 = -987422581;    double HEOjkkZacoEUnvATsuvF92707007 = -477256562;    double HEOjkkZacoEUnvATsuvF73100539 = -432696278;    double HEOjkkZacoEUnvATsuvF17022352 = -542919054;    double HEOjkkZacoEUnvATsuvF23915960 = -606060667;    double HEOjkkZacoEUnvATsuvF68968130 = -628122968;    double HEOjkkZacoEUnvATsuvF44654803 = 18974859;    double HEOjkkZacoEUnvATsuvF37099323 = -513519516;    double HEOjkkZacoEUnvATsuvF36164319 = -528328247;    double HEOjkkZacoEUnvATsuvF32249261 = -69440876;    double HEOjkkZacoEUnvATsuvF92953895 = 25899022;    double HEOjkkZacoEUnvATsuvF62204517 = -293232599;    double HEOjkkZacoEUnvATsuvF47306042 = -227544891;    double HEOjkkZacoEUnvATsuvF75129403 = -31308267;    double HEOjkkZacoEUnvATsuvF92820274 = -300639980;    double HEOjkkZacoEUnvATsuvF91374086 = -19448277;    double HEOjkkZacoEUnvATsuvF51555681 = -329671173;    double HEOjkkZacoEUnvATsuvF32312653 = -996340388;    double HEOjkkZacoEUnvATsuvF84755302 = -849520624;    double HEOjkkZacoEUnvATsuvF48967260 = -140786049;    double HEOjkkZacoEUnvATsuvF10462666 = -649377057;    double HEOjkkZacoEUnvATsuvF14685875 = -216148444;    double HEOjkkZacoEUnvATsuvF22700566 = -111558287;    double HEOjkkZacoEUnvATsuvF85821211 = -470754292;    double HEOjkkZacoEUnvATsuvF95459113 = -916353889;    double HEOjkkZacoEUnvATsuvF9819878 = -187791220;    double HEOjkkZacoEUnvATsuvF37077264 = -166339572;    double HEOjkkZacoEUnvATsuvF70238822 = -297790346;    double HEOjkkZacoEUnvATsuvF5772629 = -117276681;    double HEOjkkZacoEUnvATsuvF4590628 = -303080864;    double HEOjkkZacoEUnvATsuvF93899942 = -332310935;    double HEOjkkZacoEUnvATsuvF10930293 = -935887309;    double HEOjkkZacoEUnvATsuvF1389159 = -701256822;    double HEOjkkZacoEUnvATsuvF35526193 = -666538691;    double HEOjkkZacoEUnvATsuvF32375005 = -644073532;    double HEOjkkZacoEUnvATsuvF77611261 = -967046560;    double HEOjkkZacoEUnvATsuvF52408126 = 2944078;    double HEOjkkZacoEUnvATsuvF98665804 = -208833986;    double HEOjkkZacoEUnvATsuvF91285746 = 46994691;    double HEOjkkZacoEUnvATsuvF68199297 = -75045808;    double HEOjkkZacoEUnvATsuvF80013478 = -256346315;    double HEOjkkZacoEUnvATsuvF90545176 = -934843806;    double HEOjkkZacoEUnvATsuvF70215569 = -486554365;    double HEOjkkZacoEUnvATsuvF16945707 = -489422498;    double HEOjkkZacoEUnvATsuvF68765074 = -984846349;    double HEOjkkZacoEUnvATsuvF61958760 = -523114378;    double HEOjkkZacoEUnvATsuvF59301980 = 43961666;    double HEOjkkZacoEUnvATsuvF94327464 = -20563512;    double HEOjkkZacoEUnvATsuvF56761826 = -850273708;    double HEOjkkZacoEUnvATsuvF74778626 = -481158986;    double HEOjkkZacoEUnvATsuvF86696880 = -508756360;    double HEOjkkZacoEUnvATsuvF33660984 = -352256304;    double HEOjkkZacoEUnvATsuvF60781504 = -725418714;    double HEOjkkZacoEUnvATsuvF11833033 = -994653678;    double HEOjkkZacoEUnvATsuvF4513773 = -396466749;    double HEOjkkZacoEUnvATsuvF37733853 = -671274137;    double HEOjkkZacoEUnvATsuvF70006441 = -39927387;     HEOjkkZacoEUnvATsuvF97808673 = HEOjkkZacoEUnvATsuvF59924135;     HEOjkkZacoEUnvATsuvF59924135 = HEOjkkZacoEUnvATsuvF63138542;     HEOjkkZacoEUnvATsuvF63138542 = HEOjkkZacoEUnvATsuvF16882752;     HEOjkkZacoEUnvATsuvF16882752 = HEOjkkZacoEUnvATsuvF67243837;     HEOjkkZacoEUnvATsuvF67243837 = HEOjkkZacoEUnvATsuvF47514359;     HEOjkkZacoEUnvATsuvF47514359 = HEOjkkZacoEUnvATsuvF21922729;     HEOjkkZacoEUnvATsuvF21922729 = HEOjkkZacoEUnvATsuvF73635541;     HEOjkkZacoEUnvATsuvF73635541 = HEOjkkZacoEUnvATsuvF38107165;     HEOjkkZacoEUnvATsuvF38107165 = HEOjkkZacoEUnvATsuvF21065763;     HEOjkkZacoEUnvATsuvF21065763 = HEOjkkZacoEUnvATsuvF71895533;     HEOjkkZacoEUnvATsuvF71895533 = HEOjkkZacoEUnvATsuvF47540180;     HEOjkkZacoEUnvATsuvF47540180 = HEOjkkZacoEUnvATsuvF19622392;     HEOjkkZacoEUnvATsuvF19622392 = HEOjkkZacoEUnvATsuvF16427901;     HEOjkkZacoEUnvATsuvF16427901 = HEOjkkZacoEUnvATsuvF6534132;     HEOjkkZacoEUnvATsuvF6534132 = HEOjkkZacoEUnvATsuvF19820922;     HEOjkkZacoEUnvATsuvF19820922 = HEOjkkZacoEUnvATsuvF71925341;     HEOjkkZacoEUnvATsuvF71925341 = HEOjkkZacoEUnvATsuvF42843190;     HEOjkkZacoEUnvATsuvF42843190 = HEOjkkZacoEUnvATsuvF59873580;     HEOjkkZacoEUnvATsuvF59873580 = HEOjkkZacoEUnvATsuvF56467099;     HEOjkkZacoEUnvATsuvF56467099 = HEOjkkZacoEUnvATsuvF62189451;     HEOjkkZacoEUnvATsuvF62189451 = HEOjkkZacoEUnvATsuvF50502631;     HEOjkkZacoEUnvATsuvF50502631 = HEOjkkZacoEUnvATsuvF84794731;     HEOjkkZacoEUnvATsuvF84794731 = HEOjkkZacoEUnvATsuvF70318268;     HEOjkkZacoEUnvATsuvF70318268 = HEOjkkZacoEUnvATsuvF25508666;     HEOjkkZacoEUnvATsuvF25508666 = HEOjkkZacoEUnvATsuvF15688157;     HEOjkkZacoEUnvATsuvF15688157 = HEOjkkZacoEUnvATsuvF15201707;     HEOjkkZacoEUnvATsuvF15201707 = HEOjkkZacoEUnvATsuvF37167427;     HEOjkkZacoEUnvATsuvF37167427 = HEOjkkZacoEUnvATsuvF24668281;     HEOjkkZacoEUnvATsuvF24668281 = HEOjkkZacoEUnvATsuvF27644500;     HEOjkkZacoEUnvATsuvF27644500 = HEOjkkZacoEUnvATsuvF6379889;     HEOjkkZacoEUnvATsuvF6379889 = HEOjkkZacoEUnvATsuvF49194967;     HEOjkkZacoEUnvATsuvF49194967 = HEOjkkZacoEUnvATsuvF61718969;     HEOjkkZacoEUnvATsuvF61718969 = HEOjkkZacoEUnvATsuvF24163278;     HEOjkkZacoEUnvATsuvF24163278 = HEOjkkZacoEUnvATsuvF6608023;     HEOjkkZacoEUnvATsuvF6608023 = HEOjkkZacoEUnvATsuvF69456867;     HEOjkkZacoEUnvATsuvF69456867 = HEOjkkZacoEUnvATsuvF49582100;     HEOjkkZacoEUnvATsuvF49582100 = HEOjkkZacoEUnvATsuvF66152712;     HEOjkkZacoEUnvATsuvF66152712 = HEOjkkZacoEUnvATsuvF38252562;     HEOjkkZacoEUnvATsuvF38252562 = HEOjkkZacoEUnvATsuvF65973637;     HEOjkkZacoEUnvATsuvF65973637 = HEOjkkZacoEUnvATsuvF45536806;     HEOjkkZacoEUnvATsuvF45536806 = HEOjkkZacoEUnvATsuvF60800292;     HEOjkkZacoEUnvATsuvF60800292 = HEOjkkZacoEUnvATsuvF14976438;     HEOjkkZacoEUnvATsuvF14976438 = HEOjkkZacoEUnvATsuvF52419727;     HEOjkkZacoEUnvATsuvF52419727 = HEOjkkZacoEUnvATsuvF92707007;     HEOjkkZacoEUnvATsuvF92707007 = HEOjkkZacoEUnvATsuvF73100539;     HEOjkkZacoEUnvATsuvF73100539 = HEOjkkZacoEUnvATsuvF17022352;     HEOjkkZacoEUnvATsuvF17022352 = HEOjkkZacoEUnvATsuvF23915960;     HEOjkkZacoEUnvATsuvF23915960 = HEOjkkZacoEUnvATsuvF68968130;     HEOjkkZacoEUnvATsuvF68968130 = HEOjkkZacoEUnvATsuvF44654803;     HEOjkkZacoEUnvATsuvF44654803 = HEOjkkZacoEUnvATsuvF37099323;     HEOjkkZacoEUnvATsuvF37099323 = HEOjkkZacoEUnvATsuvF36164319;     HEOjkkZacoEUnvATsuvF36164319 = HEOjkkZacoEUnvATsuvF32249261;     HEOjkkZacoEUnvATsuvF32249261 = HEOjkkZacoEUnvATsuvF92953895;     HEOjkkZacoEUnvATsuvF92953895 = HEOjkkZacoEUnvATsuvF62204517;     HEOjkkZacoEUnvATsuvF62204517 = HEOjkkZacoEUnvATsuvF47306042;     HEOjkkZacoEUnvATsuvF47306042 = HEOjkkZacoEUnvATsuvF75129403;     HEOjkkZacoEUnvATsuvF75129403 = HEOjkkZacoEUnvATsuvF92820274;     HEOjkkZacoEUnvATsuvF92820274 = HEOjkkZacoEUnvATsuvF91374086;     HEOjkkZacoEUnvATsuvF91374086 = HEOjkkZacoEUnvATsuvF51555681;     HEOjkkZacoEUnvATsuvF51555681 = HEOjkkZacoEUnvATsuvF32312653;     HEOjkkZacoEUnvATsuvF32312653 = HEOjkkZacoEUnvATsuvF84755302;     HEOjkkZacoEUnvATsuvF84755302 = HEOjkkZacoEUnvATsuvF48967260;     HEOjkkZacoEUnvATsuvF48967260 = HEOjkkZacoEUnvATsuvF10462666;     HEOjkkZacoEUnvATsuvF10462666 = HEOjkkZacoEUnvATsuvF14685875;     HEOjkkZacoEUnvATsuvF14685875 = HEOjkkZacoEUnvATsuvF22700566;     HEOjkkZacoEUnvATsuvF22700566 = HEOjkkZacoEUnvATsuvF85821211;     HEOjkkZacoEUnvATsuvF85821211 = HEOjkkZacoEUnvATsuvF95459113;     HEOjkkZacoEUnvATsuvF95459113 = HEOjkkZacoEUnvATsuvF9819878;     HEOjkkZacoEUnvATsuvF9819878 = HEOjkkZacoEUnvATsuvF37077264;     HEOjkkZacoEUnvATsuvF37077264 = HEOjkkZacoEUnvATsuvF70238822;     HEOjkkZacoEUnvATsuvF70238822 = HEOjkkZacoEUnvATsuvF5772629;     HEOjkkZacoEUnvATsuvF5772629 = HEOjkkZacoEUnvATsuvF4590628;     HEOjkkZacoEUnvATsuvF4590628 = HEOjkkZacoEUnvATsuvF93899942;     HEOjkkZacoEUnvATsuvF93899942 = HEOjkkZacoEUnvATsuvF10930293;     HEOjkkZacoEUnvATsuvF10930293 = HEOjkkZacoEUnvATsuvF1389159;     HEOjkkZacoEUnvATsuvF1389159 = HEOjkkZacoEUnvATsuvF35526193;     HEOjkkZacoEUnvATsuvF35526193 = HEOjkkZacoEUnvATsuvF32375005;     HEOjkkZacoEUnvATsuvF32375005 = HEOjkkZacoEUnvATsuvF77611261;     HEOjkkZacoEUnvATsuvF77611261 = HEOjkkZacoEUnvATsuvF52408126;     HEOjkkZacoEUnvATsuvF52408126 = HEOjkkZacoEUnvATsuvF98665804;     HEOjkkZacoEUnvATsuvF98665804 = HEOjkkZacoEUnvATsuvF91285746;     HEOjkkZacoEUnvATsuvF91285746 = HEOjkkZacoEUnvATsuvF68199297;     HEOjkkZacoEUnvATsuvF68199297 = HEOjkkZacoEUnvATsuvF80013478;     HEOjkkZacoEUnvATsuvF80013478 = HEOjkkZacoEUnvATsuvF90545176;     HEOjkkZacoEUnvATsuvF90545176 = HEOjkkZacoEUnvATsuvF70215569;     HEOjkkZacoEUnvATsuvF70215569 = HEOjkkZacoEUnvATsuvF16945707;     HEOjkkZacoEUnvATsuvF16945707 = HEOjkkZacoEUnvATsuvF68765074;     HEOjkkZacoEUnvATsuvF68765074 = HEOjkkZacoEUnvATsuvF61958760;     HEOjkkZacoEUnvATsuvF61958760 = HEOjkkZacoEUnvATsuvF59301980;     HEOjkkZacoEUnvATsuvF59301980 = HEOjkkZacoEUnvATsuvF94327464;     HEOjkkZacoEUnvATsuvF94327464 = HEOjkkZacoEUnvATsuvF56761826;     HEOjkkZacoEUnvATsuvF56761826 = HEOjkkZacoEUnvATsuvF74778626;     HEOjkkZacoEUnvATsuvF74778626 = HEOjkkZacoEUnvATsuvF86696880;     HEOjkkZacoEUnvATsuvF86696880 = HEOjkkZacoEUnvATsuvF33660984;     HEOjkkZacoEUnvATsuvF33660984 = HEOjkkZacoEUnvATsuvF60781504;     HEOjkkZacoEUnvATsuvF60781504 = HEOjkkZacoEUnvATsuvF11833033;     HEOjkkZacoEUnvATsuvF11833033 = HEOjkkZacoEUnvATsuvF4513773;     HEOjkkZacoEUnvATsuvF4513773 = HEOjkkZacoEUnvATsuvF37733853;     HEOjkkZacoEUnvATsuvF37733853 = HEOjkkZacoEUnvATsuvF70006441;     HEOjkkZacoEUnvATsuvF70006441 = HEOjkkZacoEUnvATsuvF97808673;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZJVMThtRhBnidigSzilG51988225() {     double ppvilHuIQdBBtNxxqota23393951 = -564960488;    double ppvilHuIQdBBtNxxqota56321278 = 81717844;    double ppvilHuIQdBBtNxxqota96017718 = -647404637;    double ppvilHuIQdBBtNxxqota62604681 = -656523369;    double ppvilHuIQdBBtNxxqota36585842 = -228346499;    double ppvilHuIQdBBtNxxqota64523636 = -810387367;    double ppvilHuIQdBBtNxxqota27243574 = -134960605;    double ppvilHuIQdBBtNxxqota88645665 = -958839383;    double ppvilHuIQdBBtNxxqota63436458 = -593964423;    double ppvilHuIQdBBtNxxqota40224146 = -665592579;    double ppvilHuIQdBBtNxxqota86229086 = -80849547;    double ppvilHuIQdBBtNxxqota82857836 = -973410524;    double ppvilHuIQdBBtNxxqota96203169 = -596949231;    double ppvilHuIQdBBtNxxqota7987618 = -545898738;    double ppvilHuIQdBBtNxxqota62803019 = -306757308;    double ppvilHuIQdBBtNxxqota27541965 = -984895510;    double ppvilHuIQdBBtNxxqota71723457 = -506203736;    double ppvilHuIQdBBtNxxqota63959094 = -757785757;    double ppvilHuIQdBBtNxxqota21338301 = -463881483;    double ppvilHuIQdBBtNxxqota17628566 = -485266022;    double ppvilHuIQdBBtNxxqota63198717 = -298396329;    double ppvilHuIQdBBtNxxqota52005738 = -132115426;    double ppvilHuIQdBBtNxxqota88558529 = -675588218;    double ppvilHuIQdBBtNxxqota72032311 = -303152101;    double ppvilHuIQdBBtNxxqota80411805 = -97248342;    double ppvilHuIQdBBtNxxqota45177293 = -416203581;    double ppvilHuIQdBBtNxxqota97589615 = -224879111;    double ppvilHuIQdBBtNxxqota1066227 = 67168992;    double ppvilHuIQdBBtNxxqota32545493 = -573665306;    double ppvilHuIQdBBtNxxqota80966990 = -357272883;    double ppvilHuIQdBBtNxxqota28426523 = -893529643;    double ppvilHuIQdBBtNxxqota86384899 = -932659208;    double ppvilHuIQdBBtNxxqota82906021 = -406768463;    double ppvilHuIQdBBtNxxqota77750349 = -972491948;    double ppvilHuIQdBBtNxxqota8013011 = -985019136;    double ppvilHuIQdBBtNxxqota74248717 = -669466494;    double ppvilHuIQdBBtNxxqota99227484 = -631425632;    double ppvilHuIQdBBtNxxqota41725399 = -449322087;    double ppvilHuIQdBBtNxxqota79176140 = -296765855;    double ppvilHuIQdBBtNxxqota74757311 = -818567010;    double ppvilHuIQdBBtNxxqota92688087 = -49999527;    double ppvilHuIQdBBtNxxqota4809883 = -644478231;    double ppvilHuIQdBBtNxxqota64176657 = -74450187;    double ppvilHuIQdBBtNxxqota23213006 = -355838180;    double ppvilHuIQdBBtNxxqota84687883 = -416277679;    double ppvilHuIQdBBtNxxqota39500862 = -955243816;    double ppvilHuIQdBBtNxxqota16790807 = -575387914;    double ppvilHuIQdBBtNxxqota32028731 = -799243514;    double ppvilHuIQdBBtNxxqota19779450 = -727799195;    double ppvilHuIQdBBtNxxqota98464798 = -929048294;    double ppvilHuIQdBBtNxxqota47235570 = -762178118;    double ppvilHuIQdBBtNxxqota63422899 = -706407427;    double ppvilHuIQdBBtNxxqota50322864 = -637339425;    double ppvilHuIQdBBtNxxqota61852804 = -289164759;    double ppvilHuIQdBBtNxxqota88789508 = -768323017;    double ppvilHuIQdBBtNxxqota71388212 = -332845062;    double ppvilHuIQdBBtNxxqota67762748 = -242693938;    double ppvilHuIQdBBtNxxqota23985407 = -244252537;    double ppvilHuIQdBBtNxxqota82192876 = -459275028;    double ppvilHuIQdBBtNxxqota91408549 = -812142919;    double ppvilHuIQdBBtNxxqota66934020 = -485508257;    double ppvilHuIQdBBtNxxqota26177347 = -102129598;    double ppvilHuIQdBBtNxxqota56100172 = -285174078;    double ppvilHuIQdBBtNxxqota82469467 = -136691540;    double ppvilHuIQdBBtNxxqota11797623 = -772062937;    double ppvilHuIQdBBtNxxqota99844187 = -148190339;    double ppvilHuIQdBBtNxxqota99951815 = -466642062;    double ppvilHuIQdBBtNxxqota18452821 = -624457283;    double ppvilHuIQdBBtNxxqota99974606 = -560879602;    double ppvilHuIQdBBtNxxqota88554301 = -637290814;    double ppvilHuIQdBBtNxxqota28314480 = -253469879;    double ppvilHuIQdBBtNxxqota29998058 = 43118350;    double ppvilHuIQdBBtNxxqota84782953 = -361019903;    double ppvilHuIQdBBtNxxqota46580990 = -645314473;    double ppvilHuIQdBBtNxxqota24940479 = -335266495;    double ppvilHuIQdBBtNxxqota58388834 = -653918099;    double ppvilHuIQdBBtNxxqota87829080 = 42334761;    double ppvilHuIQdBBtNxxqota65345523 = -219750038;    double ppvilHuIQdBBtNxxqota87344428 = -886874422;    double ppvilHuIQdBBtNxxqota40910943 = -142004526;    double ppvilHuIQdBBtNxxqota28386487 = -840815667;    double ppvilHuIQdBBtNxxqota65560885 = -425635597;    double ppvilHuIQdBBtNxxqota81286777 = -205031814;    double ppvilHuIQdBBtNxxqota34080695 = -644617012;    double ppvilHuIQdBBtNxxqota33731420 = -595094765;    double ppvilHuIQdBBtNxxqota65003624 = -87122216;    double ppvilHuIQdBBtNxxqota36062035 = -195319783;    double ppvilHuIQdBBtNxxqota21053218 = -17603705;    double ppvilHuIQdBBtNxxqota88960840 = -104168932;    double ppvilHuIQdBBtNxxqota36624798 = -552174075;    double ppvilHuIQdBBtNxxqota6485970 = -326772556;    double ppvilHuIQdBBtNxxqota75242078 = -287173096;    double ppvilHuIQdBBtNxxqota59532523 = -990047060;    double ppvilHuIQdBBtNxxqota87767590 = -484622937;    double ppvilHuIQdBBtNxxqota7823291 = -233058754;    double ppvilHuIQdBBtNxxqota66510740 = -947869929;    double ppvilHuIQdBBtNxxqota48709711 = -259304153;    double ppvilHuIQdBBtNxxqota81707190 = -937758647;    double ppvilHuIQdBBtNxxqota11415384 = -583775243;    double ppvilHuIQdBBtNxxqota84843696 = -564960488;     ppvilHuIQdBBtNxxqota23393951 = ppvilHuIQdBBtNxxqota56321278;     ppvilHuIQdBBtNxxqota56321278 = ppvilHuIQdBBtNxxqota96017718;     ppvilHuIQdBBtNxxqota96017718 = ppvilHuIQdBBtNxxqota62604681;     ppvilHuIQdBBtNxxqota62604681 = ppvilHuIQdBBtNxxqota36585842;     ppvilHuIQdBBtNxxqota36585842 = ppvilHuIQdBBtNxxqota64523636;     ppvilHuIQdBBtNxxqota64523636 = ppvilHuIQdBBtNxxqota27243574;     ppvilHuIQdBBtNxxqota27243574 = ppvilHuIQdBBtNxxqota88645665;     ppvilHuIQdBBtNxxqota88645665 = ppvilHuIQdBBtNxxqota63436458;     ppvilHuIQdBBtNxxqota63436458 = ppvilHuIQdBBtNxxqota40224146;     ppvilHuIQdBBtNxxqota40224146 = ppvilHuIQdBBtNxxqota86229086;     ppvilHuIQdBBtNxxqota86229086 = ppvilHuIQdBBtNxxqota82857836;     ppvilHuIQdBBtNxxqota82857836 = ppvilHuIQdBBtNxxqota96203169;     ppvilHuIQdBBtNxxqota96203169 = ppvilHuIQdBBtNxxqota7987618;     ppvilHuIQdBBtNxxqota7987618 = ppvilHuIQdBBtNxxqota62803019;     ppvilHuIQdBBtNxxqota62803019 = ppvilHuIQdBBtNxxqota27541965;     ppvilHuIQdBBtNxxqota27541965 = ppvilHuIQdBBtNxxqota71723457;     ppvilHuIQdBBtNxxqota71723457 = ppvilHuIQdBBtNxxqota63959094;     ppvilHuIQdBBtNxxqota63959094 = ppvilHuIQdBBtNxxqota21338301;     ppvilHuIQdBBtNxxqota21338301 = ppvilHuIQdBBtNxxqota17628566;     ppvilHuIQdBBtNxxqota17628566 = ppvilHuIQdBBtNxxqota63198717;     ppvilHuIQdBBtNxxqota63198717 = ppvilHuIQdBBtNxxqota52005738;     ppvilHuIQdBBtNxxqota52005738 = ppvilHuIQdBBtNxxqota88558529;     ppvilHuIQdBBtNxxqota88558529 = ppvilHuIQdBBtNxxqota72032311;     ppvilHuIQdBBtNxxqota72032311 = ppvilHuIQdBBtNxxqota80411805;     ppvilHuIQdBBtNxxqota80411805 = ppvilHuIQdBBtNxxqota45177293;     ppvilHuIQdBBtNxxqota45177293 = ppvilHuIQdBBtNxxqota97589615;     ppvilHuIQdBBtNxxqota97589615 = ppvilHuIQdBBtNxxqota1066227;     ppvilHuIQdBBtNxxqota1066227 = ppvilHuIQdBBtNxxqota32545493;     ppvilHuIQdBBtNxxqota32545493 = ppvilHuIQdBBtNxxqota80966990;     ppvilHuIQdBBtNxxqota80966990 = ppvilHuIQdBBtNxxqota28426523;     ppvilHuIQdBBtNxxqota28426523 = ppvilHuIQdBBtNxxqota86384899;     ppvilHuIQdBBtNxxqota86384899 = ppvilHuIQdBBtNxxqota82906021;     ppvilHuIQdBBtNxxqota82906021 = ppvilHuIQdBBtNxxqota77750349;     ppvilHuIQdBBtNxxqota77750349 = ppvilHuIQdBBtNxxqota8013011;     ppvilHuIQdBBtNxxqota8013011 = ppvilHuIQdBBtNxxqota74248717;     ppvilHuIQdBBtNxxqota74248717 = ppvilHuIQdBBtNxxqota99227484;     ppvilHuIQdBBtNxxqota99227484 = ppvilHuIQdBBtNxxqota41725399;     ppvilHuIQdBBtNxxqota41725399 = ppvilHuIQdBBtNxxqota79176140;     ppvilHuIQdBBtNxxqota79176140 = ppvilHuIQdBBtNxxqota74757311;     ppvilHuIQdBBtNxxqota74757311 = ppvilHuIQdBBtNxxqota92688087;     ppvilHuIQdBBtNxxqota92688087 = ppvilHuIQdBBtNxxqota4809883;     ppvilHuIQdBBtNxxqota4809883 = ppvilHuIQdBBtNxxqota64176657;     ppvilHuIQdBBtNxxqota64176657 = ppvilHuIQdBBtNxxqota23213006;     ppvilHuIQdBBtNxxqota23213006 = ppvilHuIQdBBtNxxqota84687883;     ppvilHuIQdBBtNxxqota84687883 = ppvilHuIQdBBtNxxqota39500862;     ppvilHuIQdBBtNxxqota39500862 = ppvilHuIQdBBtNxxqota16790807;     ppvilHuIQdBBtNxxqota16790807 = ppvilHuIQdBBtNxxqota32028731;     ppvilHuIQdBBtNxxqota32028731 = ppvilHuIQdBBtNxxqota19779450;     ppvilHuIQdBBtNxxqota19779450 = ppvilHuIQdBBtNxxqota98464798;     ppvilHuIQdBBtNxxqota98464798 = ppvilHuIQdBBtNxxqota47235570;     ppvilHuIQdBBtNxxqota47235570 = ppvilHuIQdBBtNxxqota63422899;     ppvilHuIQdBBtNxxqota63422899 = ppvilHuIQdBBtNxxqota50322864;     ppvilHuIQdBBtNxxqota50322864 = ppvilHuIQdBBtNxxqota61852804;     ppvilHuIQdBBtNxxqota61852804 = ppvilHuIQdBBtNxxqota88789508;     ppvilHuIQdBBtNxxqota88789508 = ppvilHuIQdBBtNxxqota71388212;     ppvilHuIQdBBtNxxqota71388212 = ppvilHuIQdBBtNxxqota67762748;     ppvilHuIQdBBtNxxqota67762748 = ppvilHuIQdBBtNxxqota23985407;     ppvilHuIQdBBtNxxqota23985407 = ppvilHuIQdBBtNxxqota82192876;     ppvilHuIQdBBtNxxqota82192876 = ppvilHuIQdBBtNxxqota91408549;     ppvilHuIQdBBtNxxqota91408549 = ppvilHuIQdBBtNxxqota66934020;     ppvilHuIQdBBtNxxqota66934020 = ppvilHuIQdBBtNxxqota26177347;     ppvilHuIQdBBtNxxqota26177347 = ppvilHuIQdBBtNxxqota56100172;     ppvilHuIQdBBtNxxqota56100172 = ppvilHuIQdBBtNxxqota82469467;     ppvilHuIQdBBtNxxqota82469467 = ppvilHuIQdBBtNxxqota11797623;     ppvilHuIQdBBtNxxqota11797623 = ppvilHuIQdBBtNxxqota99844187;     ppvilHuIQdBBtNxxqota99844187 = ppvilHuIQdBBtNxxqota99951815;     ppvilHuIQdBBtNxxqota99951815 = ppvilHuIQdBBtNxxqota18452821;     ppvilHuIQdBBtNxxqota18452821 = ppvilHuIQdBBtNxxqota99974606;     ppvilHuIQdBBtNxxqota99974606 = ppvilHuIQdBBtNxxqota88554301;     ppvilHuIQdBBtNxxqota88554301 = ppvilHuIQdBBtNxxqota28314480;     ppvilHuIQdBBtNxxqota28314480 = ppvilHuIQdBBtNxxqota29998058;     ppvilHuIQdBBtNxxqota29998058 = ppvilHuIQdBBtNxxqota84782953;     ppvilHuIQdBBtNxxqota84782953 = ppvilHuIQdBBtNxxqota46580990;     ppvilHuIQdBBtNxxqota46580990 = ppvilHuIQdBBtNxxqota24940479;     ppvilHuIQdBBtNxxqota24940479 = ppvilHuIQdBBtNxxqota58388834;     ppvilHuIQdBBtNxxqota58388834 = ppvilHuIQdBBtNxxqota87829080;     ppvilHuIQdBBtNxxqota87829080 = ppvilHuIQdBBtNxxqota65345523;     ppvilHuIQdBBtNxxqota65345523 = ppvilHuIQdBBtNxxqota87344428;     ppvilHuIQdBBtNxxqota87344428 = ppvilHuIQdBBtNxxqota40910943;     ppvilHuIQdBBtNxxqota40910943 = ppvilHuIQdBBtNxxqota28386487;     ppvilHuIQdBBtNxxqota28386487 = ppvilHuIQdBBtNxxqota65560885;     ppvilHuIQdBBtNxxqota65560885 = ppvilHuIQdBBtNxxqota81286777;     ppvilHuIQdBBtNxxqota81286777 = ppvilHuIQdBBtNxxqota34080695;     ppvilHuIQdBBtNxxqota34080695 = ppvilHuIQdBBtNxxqota33731420;     ppvilHuIQdBBtNxxqota33731420 = ppvilHuIQdBBtNxxqota65003624;     ppvilHuIQdBBtNxxqota65003624 = ppvilHuIQdBBtNxxqota36062035;     ppvilHuIQdBBtNxxqota36062035 = ppvilHuIQdBBtNxxqota21053218;     ppvilHuIQdBBtNxxqota21053218 = ppvilHuIQdBBtNxxqota88960840;     ppvilHuIQdBBtNxxqota88960840 = ppvilHuIQdBBtNxxqota36624798;     ppvilHuIQdBBtNxxqota36624798 = ppvilHuIQdBBtNxxqota6485970;     ppvilHuIQdBBtNxxqota6485970 = ppvilHuIQdBBtNxxqota75242078;     ppvilHuIQdBBtNxxqota75242078 = ppvilHuIQdBBtNxxqota59532523;     ppvilHuIQdBBtNxxqota59532523 = ppvilHuIQdBBtNxxqota87767590;     ppvilHuIQdBBtNxxqota87767590 = ppvilHuIQdBBtNxxqota7823291;     ppvilHuIQdBBtNxxqota7823291 = ppvilHuIQdBBtNxxqota66510740;     ppvilHuIQdBBtNxxqota66510740 = ppvilHuIQdBBtNxxqota48709711;     ppvilHuIQdBBtNxxqota48709711 = ppvilHuIQdBBtNxxqota81707190;     ppvilHuIQdBBtNxxqota81707190 = ppvilHuIQdBBtNxxqota11415384;     ppvilHuIQdBBtNxxqota11415384 = ppvilHuIQdBBtNxxqota84843696;     ppvilHuIQdBBtNxxqota84843696 = ppvilHuIQdBBtNxxqota23393951;}
// Junk Finished
