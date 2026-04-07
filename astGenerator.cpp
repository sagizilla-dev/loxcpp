#include "config.hpp"
#include "common.hpp"
#include "token.hpp"

/*
    BNF to define metasyntax for our grammar:

    expr     -> binary | literal | grouping | unary;
    binary   -> expr operator expr;
    literal  -> IDENTIFIER | STRING | NUMBER | "true" | "false" | nil;
    grouping -> "(" expr ")";
    unary    -> ("!" | "-") expr;
    operator -> "==" | "!=" | "<" | "<=" | ">" | ">=" | "+" | "-" | "*" | "/";

    To avoid ambiguity during parsing and introduce operators precedence we redefine it:

    program         -> declaration* END;
    declaration     -> varDeclaration | statement;
    varDeclaration  -> "var" IDENTIFIER ( "=" expr )? ";";
    statement       -> exprStatement | printStatement | blockStatement | ifStatement;
    ifStatement     -> "if" "(" expr ")" statement ( "else" statement )?;
    exprStatement   -> expr ";";
    printStatement  -> "print" expr ";";
    blockStatement  -> "{" declaration* "}";
    expr            -> assignment;
    assignment      -> ( IDENTIFIER "=" assignment ) | equality | logicOR;
    logicOR         -> logicAND ( "or" logicAND )*;
    logicAND        -> equality ( "and" equality )*;
    equality        -> comparison ( ( "==" | "!=" ) comparison )*;
    comparison      -> term ( ( ">" | ">=" | "<" | "<=" ) term )*;
    term            -> factor ( ( "+" | "-" ) factor )*;
    factor          -> unary ( ( "/" | "*" ) unary )*;
    unary           -> ( ("!" | "-") unary ) | primary;
    primary         -> NUMBER | STRING | "true" | "false" | nil | ( "(" expr ")" ) | IDENTIFIER;
*/

using astTypes = std::pair<std::string, std::vector<std::pair<std::string, std::string>>>;

void defineVisitor(std::ofstream& file, std::string baseName, std::vector<astTypes> types) {
    // forward declaration
    for (auto type: types) {
        file<<"class "<<type.first<<baseName<<";\n";
    }

    file<<"class "<<baseName<<"Visitor {\n";
    file<<"public:\n";
    for (auto type: types) {
        file<<"\tvirtual std::any visit"<<type.first<<baseName<<"("<<type.first<<baseName<<"* expr) = 0;\n";
    }
    file<<"};\n";
}
void defineAST(std::ofstream& file, std::string baseName, std::vector<astTypes> types) {
    // define visitor first due to forward declaration
    defineVisitor(file, baseName, types);

    file<<"class "<<baseName<<" {\n";
    file<<"public:\n";
    file<<"\tvirtual std::any accept("<<baseName<<"Visitor* visitor) = 0;\n";
    file<<"};\n";
    for (auto type: types) {
        file<<"class "<<type.first<<baseName<<": public " <<baseName<< "{\n";
        file<<"public:\n";
        // constructor
        file<<'\t'<<type.first<<baseName<<"(";
        for (int i=0; i<type.second.size(); i++) {
            file<<type.second[i].first<<" "<<type.second[i].second;
            if (i!=type.second.size()-1) {
                file<<", ";
            }
        }
        file<<"): ";
        for (int i=0; i<type.second.size(); i++) {
            file<<type.second[i].second<<"("<<type.second[i].second<<")";
            if (i!=type.second.size()-1) {
                file<<", ";
            }
        }
        file<<" {\n\t}\n";
        // fields
        for (auto field: type.second) {
            file<<'\t'<<field.first<<" "<<field.second<<";\n";
        }
        file<<"\tstd::any accept("<<baseName<<"Visitor* visitor) override {\n";
        file<<"\t\treturn visitor->visit"<<type.first<<baseName<<"(this);\n";
        file<<"\t}\n";
        file<<"};\n";
    }
}
int main() {
    std::ofstream file("../ast.hpp");
    file << "#pragma once\n";
    file << "#include \"config.hpp\"\n";
    file << "#include \"common.hpp\"\n";
    file << "#include \"token.hpp\"\n";
    defineAST(file, "Expr", {
        {"Binary", {{"Expr*", "left"}, {"Expr*", "right"}, {"Token", "op"}}},
        {"Unary", {{"Expr*", "expr"}, {"Token", "op"}}},
        {"Grouping", {{"Expr*", "expr"}}},
        {"Literal", {{"std::any", "value"}}},
        {"Variable", {{"Token", "name"}}},
        {"Assign", {{"Token", "name"}, {"Expr*", "value"}}},
        {"Logic", {{"Expr*", "left"}, {"Expr*", "right"}, {"Token", "op"}}},
    });
    defineAST(file, "Stmt", {
        {"Print", {{"Expr*", "expr"}}},
        {"Expression", {{"Expr*", "expr"}}},
        {"VarDeclaration", {{"Token", "name"}, {"Expr*", "initializer"}}},
        {"Block", {{"std::vector<Stmt*>", "statements"}}},
        {"If", {{"Expr*", "condition"}, {"Stmt*", "thenStatement"}, {"Stmt*", "elseStatement"}}}
    });
}