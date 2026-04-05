#pragma once
#include "config.hpp"
#include "token.hpp"

class ScannerError: public std::exception {
public:
    int line;
    std::string message;
    std::string errorMessage;
    static bool errorFound;
    ScannerError(int line, std::string errorMessage): line(line), errorMessage(errorMessage) {
        errorFound = true;
        message = "Scanner error[" + std::to_string(line)+"] -> " + errorMessage;
    }
    const char* what() const noexcept override {
        return message.c_str();
    }
};
bool ScannerError::errorFound = false;

struct Scanner {
    std::string code;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0; // points to the next char to consume
    int line = 1;
    std::unordered_map<std::string, TOKEN_TYPE> keywords = {
        {"and", AND},
        {"class", CLASS},
        {"else", ELSE},
        {"false", FALSE},
        {"fun", FUN},
        {"for", FOR},
        {"if", IF},
        {"nil", NIL},
        {"or", OR},
        {"print", PRINT},
        {"return", RETURN},
        {"super", SUPER},
        {"this", THIS},
        {"true", TRUE},
        {"var", VAR},
        {"while", WHILE},
    };
    Scanner(std::string code): code(code) {

    }
    void scan() {
        try {
            while (!isAtEnd()) {
                start = current;
                scanToken();
            }
            tokens.push_back(Token(END, "", std::any{}, line));
        } catch (const ScannerError& e) {
            std::cout<<e.what()<<'\n';
        }
    }
    void scanToken() {
        char c = advance();
        switch (c) {
            case ('('): {
                addToken(LEFT_PAREN);
                break;
            }
            case (')'): {
                addToken(RIGHT_PAREN);
                break;
            }
            case ('{'): {
                addToken(LEFT_BRACE);
                break;
            }
            case ('}'): {
                addToken(RIGHT_BRACE);
                break;
            }
            case (','): {
                addToken(COMMA);
                break;
            }
            case ('.'): {
                addToken(DOT);
                break;
            }
            case ('-'): {
                addToken(MINUS);
                break;
            }
            case ('+'): {
                addToken(PLUS);
                break;
            }
            case (';'): {
                addToken(SEMICOLON);
                break;
            }
            case ('*'): {
                addToken(STAR);
                break;
            }
            case ' ':
            case '\r':
            case '\t': {
                break;
            }
            case '\n': {
                line++;
                break;
            }
            case ('/'): {
                if (match('/')) {
                    while (!isAtEnd() && peek()!='\n') {
                        advance();
                    }
                } else if (match('*')) {
                    while (!isAtEnd() && !(peek()=='*' && peekNext()=='/')) {
                        if (peek()=='\n') {
                            line++;
                        }
                        advance();
                    }
                    advance(); // consume the *
                    advance(); // consume the /
                    if (isAtEnd()) {
                        std::string errorMessage = "Unterminated comment block";
                        throw ScannerError(line, errorMessage);
                    }
                } else {
                    addToken(SLASH);
                }
                break;
            }
            case ('!'): {
                if (match('=')) {
                    addToken(BANG_EQUAL);
                }
                addToken(BANG);
                break;
            }
            case ('='): {
                if (match('=')) {
                    addToken(EQUAL_EQUAL);
                }
                addToken(EQUAL);
                break;
            }
            case ('<'): {
                if (match('=')) {
                    addToken(LESS_EQUAL);
                }
                addToken(LESS);
                break;
            }
            case ('>'): {
                if (match('=')) {
                    addToken(GREATER_EQUAL);
                }
                addToken(GREATER);
                break;
            }
            case ('"'): {
                createString();
                break;
            }
            default: {
                if (isdigit(c)) {
                    createNumber();
                    break;
                } else if (isalpha(c)) {
                    createIdentifier();
                    break;
                }
                std::string errorMessage = "Undefined symbol: ";
                errorMessage+=code.substr(start, current-start);
                throw ScannerError(line, errorMessage);
            }
               
        }
    }
    void createIdentifier() {
        while (!isAtEnd() && isalnum(peek())) {
            advance();
        }
        if (keywords.count(code.substr(start, current-start))) {
            addToken(keywords[code.substr(start, current-start)]);
        } else {
            addToken(IDENTIFIER);
        }
    }
    void createNumber() {
        while (!isAtEnd() && isdigit(peek())) {
            advance();
        }
        if (peek()=='.' && isdigit(peekNext())) {
            advance(); // consume the .
            while (isdigit(peek())) {
                advance();
            }
        }
        addToken(NUMBER, std::stod(code.substr(start, current-start)));
    }
    void createString() {
        while (!isAtEnd() && peek()!='"') {
            if (peek()=='\n') {
                std::string errorMessage = "Unterminated string: " + code.substr(start, current-start);
                throw ScannerError(line, errorMessage);
            }
            advance();
        }
        if (isAtEnd()) {
            std::string errorMessage = "Unterminated string: " + code.substr(start, current-start);
            throw ScannerError(line, errorMessage);
        }
        advance(); // consume the closing "
        std::string literal = code.substr(start+1, current-start-2);
        addToken(STRING, literal);
    }
    void addToken(TOKEN_TYPE type) {
        addToken(type, std::any{});
    }
    void addToken(TOKEN_TYPE type, std::any literal) {
        std::string lexeme = code.substr(start, current-start);
        tokens.push_back(Token(type, lexeme, literal, line));
    }
    bool isAtEnd() {
        return current >= code.length();
    }
    char advance() {
        return code[current++];
    }
    char peek() {
        if (isAtEnd()) {
            return '\0';
        }
        return code[current];
    }
    char peekNext() {
        if (current+1>=code.length()) {
            return '\0';
        }
        return code[current+1];
    }
    bool match(char c) {
        if (isAtEnd()) {
            return false;
        }
        if (code[current]==c) {
            current++;
            return true;
        }
        return false;
    }
};