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
*/

using astTypes = std::pair<std::string, std::vector<std::pair<std::string, std::string>>>;

void defineAST(std::ofstream& file, std::string baseName, std::vector<astTypes> types) {
    file<<"class "<<baseName<<" {\n";
    file<<"public:\n";
    file<<"virtual std::any accept(Visitor* visitor) = 0;\n";
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
        file<<"\tstd::any accept(Visitor* visitor) override {\n";
        file<<"\t\treturn visitor->visit"<<type.first<<baseName<<"(this);\n";
        file<<"\t}\n";
        file<<"};\n";
    }
}
void defineVisitor(std::ofstream& file, std::string baseName, std::vector<astTypes> types) {
    // forward declaration
    for (auto type: types) {
        file<<"class "<<type.first<<baseName<<";\n";
    }

    file<<"class Visitor {\n";
    file<<"public:\n";
    for (auto type: types) {
        file<<"\tvirtual std::any visit"<<type.first<<baseName<<"("<<type.first<<baseName<<"* expr) = 0;\n";
    }
    file<<"};\n";
}
int main() {
    std::ofstream file("../ast.hpp");
    file << "#pragma once\n";
    file << "#include \"config.hpp\"\n";
    file << "#include \"common.hpp\"\n";
    file << "#include \"token.hpp\"\n";
    defineVisitor(file, "Expr", {
        {"Binary", {{"Expr*", "left"}, {"Expr*", "right"}, {"Token*", "op"}}},
        {"Unary", {{"Expr*", "expr"}, {"Token*", "op"}}},
        {"Grouping", {{"Expr*", "expr"}}},
        {"Literal", {{"std::any", "value"}}},
    });
    defineAST(file, "Expr", {
        {"Binary", {{"Expr*", "left"}, {"Expr*", "right"}, {"Token*", "op"}}},
        {"Unary", {{"Expr*", "expr"}, {"Token*", "op"}}},
        {"Grouping", {{"Expr*", "expr"}}},
        {"Literal", {{"std::any", "value"}}},
    });
}