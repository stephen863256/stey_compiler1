#include "ast.hpp"

#include <cstring>
#include <iostream>
#include <stack>

#define _AST_NODE_ERROR_                                                       \
    std::cerr << "Abort due to node cast error."                               \
                 "Contact with TAs to solve your problem."                     \
              << std::endl;                                                    \
    std::abort();
#define _STR_EQ(a, b) (strcmp((a), (b)) == 0)

void AST::run_visitor(ASTVisitor &visitor) { root->accept(visitor); }

AST::AST(syntax_tree *s) {
    if (s == nullptr) {
        std::cerr << "empty input tree!" << std::endl;
        std::abort();
    }
    auto node = transform_node_iter(s->root);
    del_syntax_tree(s);
    root = std::shared_ptr<ASTCompUnit>(static_cast<ASTCompUnit *>(node));
}

ASTNode *AST::transform_node_iter(syntax_tree_node *n) {
    /*
    CompUnit : Decl { $$ = node("CompUnit", 1, $1);gt->root = $$;}
         | CompUnit Decl { $$ = node("CompUnit", 2, $1, $2); }
         ;
    */
    if(_STR_EQ(n->name, "CompUnit")) {
        auto node = new ASTCompUnit();

        std::stack<syntax_tree_node *> s;
        auto list_ptr = n;
        while (list_ptr->children_num == 2) {
            s.push(list_ptr->children[1]);
            list_ptr = list_ptr->children[0];
        }
        s.push(list_ptr->children[0]);

        while (!s.empty()) {
            auto child_node = static_cast<ASTDecl *>(transform_node_iter(s.top()));
            if(child_node == nullptr){
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTDecl>(child_node);
            node->declarations.push_back(child_node_ptr);
            s.pop();
        }
        return node;        
    }
    /*
    Decl : ConstDecl { $$ = node("Decl", 1, $1); }
        | VarDecl { $$ = node("Decl", 1, $1); }
        | FuncDef { $$ = node("Decl", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name, "Decl" )){
        auto node = new ASTDecl();
        if(n->children[0]->name == "ConstDecl"){
            auto const_decl = static_cast<ASTConstDecl *>(transform_node_iter(n->children[0]));
            node->const_decl = std::shared_ptr<ASTConstDecl>(const_decl);
        }
        else if(n->children[0]->name == "VarDecl"){
            auto var_decl = static_cast<ASTVarDecl *>(transform_node_iter(n->children[0]));
            node->var_decl = std::shared_ptr<ASTVarDecl>(var_decl);
        }
        else if(n->children[0]->name == "FuncDef"){
            auto func_def = static_cast<ASTFuncDef *>(transform_node_iter(n->children[0]));
            node->func_def = std::shared_ptr<ASTFuncDef>(func_def);
        }
        else{
            _AST_NODE_ERROR_
        }       
        return node;
    }
    /*
    ConstDecl : CONST BType ConstDefList SEMICOLON { $$ = node("ConstDecl", 4,$1, $2, $3, $4); }
        ;
    
    ConstDefList : ConstDefList COMMA ConstDef { $$ = node("ConstDef", 3, $1, $2, $3); }
        | ConstDef { $$ = node("ConstDef", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name, "ConstDecl")){
        auto node = new ASTConstDecl();
        if(n->children[1]->name == "BType")
        {
            if(n->children[1]->children[0]->name == "int"){
                node->type = TYPE_INT;
            }
            else if(n->children[1]->children[0]->name == "float"){
                node->type = TYPE_FLOAT;
            }
            else{
                _AST_NODE_ERROR_
            }        
        }
        auto list_ptr = n->children[2];
        std::stack<syntax_tree_node *> s;
        if(list_ptr->children_num == 3)
        {
            while(list_ptr->children_num == 3)
            {
                s.push(list_ptr->children[2]);
                list_ptr = list_ptr->children[0];
            }
        }
        s.push(list_ptr->children[0]);

        while (!s.empty()) {
            auto child_node = static_cast<ASTConstDef *>(transform_node_iter(s.top()));
            if(child_node == nullptr){
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTConstDef>(child_node);
            node->const_defs.push_back(child_node_ptr);
            s.pop();
        }
        return node;
    }
    /*
    ConstDef : ID ASSIGN ConstInitVal { $$ = node("ConstDef", 3, $1, $2, $3); }
        |  ID ConstArrayIdent ASSIGN ConstInitVal { $$ = node("ConstDef", 4, $1, $2, $3,$4);}
        ;
    */
    else if(_STR_EQ(n->name, "ConstDef")){
        auto node = new ASTConstDef();
        node->id = n->children[0]->name;
        if(n->children[1]->name == "ConstArrayIdent"){
            auto const_array_ident = static_cast<ASTConstArrayIdent *>(transform_node_iter(n->children[
                0]));
            node->const_array_ident = std::shared_ptr<ASTConstArrayIdent>(const_array_ident);
            auto const_init_val = static_cast<ASTConstInitVal *>(transform_node_iter(n->children[3]));
            node->const_init_val = std::shared_ptr<ASTConstInitVal>(const_init_val);
        }
        else{
            auto const_init_val = static_cast<ASTConstInitVal *>(transform_node_iter(n->children[2]));
            node->const_init_val = std::shared_ptr<ASTConstInitVal>(const_init_val);
        }
        return node;
    }
    /*
    ConstArrayIdent : empty { $$ = node("ConstArrayIdent", 0); }
        | ConstArrayIdent LBRACKET constExp RBRACKET { $$ = node("ConstArrayIdent", 4, $1, $2, $3, $4); }
        ;
    */
    else if(_STR_EQ(n->name, "ConstArrayIdent")){
        auto node = new ASTConstArrayIdent();
        auto list_ptr = n;
        std::stack<syntax_tree_node *> s;
        if(list_ptr->children_num == 4){
            while(list_ptr->children_num == 4){
                s.push(list_ptr->children[2]);
                list_ptr = list_ptr->children[0];
            }
        }

        while(!s.empty()){
            auto child_node = static_cast<ASTConstExp *>(transform_node_iter(s.top()));
            if(child_node == nullptr){
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTConstExp>(child_node);
            node->const_exps.push_back(child_node_ptr);
            s.pop();
        }
        return node;
    }
    /*
    ConstInitVal : ConstExp { $$ = node("ConstInitVal", 1, $1); }
        | LBRACE ConstInitValArrayList RBRACE { $$ = node("ConstInitVal", 3,$1, $2, $3); }
        | LBRACE RBRACE { $$ = node("ConstInitVal", 2, $1, $2); }
        ;

    ConstInitValArrayList : ConstInitValArrayList COMMA ConstInitVal { $$ = node("ConstInitValArrayList", 3, $1, $2, $3); }
        | ConstInitVal { $$ = node("ConstInitValArrayList", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name, "ConstInitVal")){
        auto node = new ASTConstInitVal();
        if(n->children[0]->name == "ConstExp"){
            auto const_exp = static_cast<ASTConstExp *>(transform_node_iter(n->children[0]));
            node->const_exp = std::shared_ptr<ASTConstExp>(const_exp);
        }
        else if(n->children[1]->name == "ConstInitValArrayList"){
            auto list_ptr = n->children[1];
            std::stack<syntax_tree_node *> s;
            if(list_ptr->children_num == 3){
                while(list_ptr->children_num == 3){
                    s.push(list_ptr->children[2]);
                    list_ptr = list_ptr->children[0];
                }
            }
            s.push(list_ptr->children[0]);  
            while(!s.empty()){
                auto child_node = static_cast<ASTConstInitVal *>(transform_node_iter(s.top()));
                if(child_node == nullptr){
                    _AST_NODE_ERROR_;
                }
                auto child_node_ptr = std::shared_ptr<ASTConstInitVal>(child_node);
                node->const_init_vals.push_back(child_node_ptr);
                s.pop();
            }
        }
        return node;
    }
    /*
    VarDecl : BType VarDefList SEMICOLON { $$ = node("VarDecl", 3, $1, $2, $3); }

    VarDefList : VarDefList COMMA VarDef { $$ = node("VarDefList", 3, $1, $2, $3); }
        | VarDef { $$ = node("VarDefList", 1, $1); }
        ;
    BType : INT { $$ = node("BType", 1, $1); }
        | FLOAT { $$ = node("BType", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"VarDecl"))
    {
        auto node = new ASTVarDecl();
        if(n->children[0]->children[0]->name == "int")
            node->type = TYPE_INT;
        else if(n->children[0]->children[0]->name == "float")
            node->type = TYPE_FLOAT;
        else{
            _AST_NODE_ERROR_
        }          
        auto list_ptr = n->children[1];
        std::stack<syntax_tree_node *> s;
        if(list_ptr->children_num == 3)
        {
            while(list_ptr->children_num == 3)
            {
                s.push(list_ptr->children[2]);
                list_ptr = list_ptr->children[0];
            }
        }
        s.push(list_ptr->children[0]);

        while (!s.empty()) {
            auto child_node = static_cast<ASTVarDef *>(transform_node_iter(s.top()));
            if(child_node == nullptr){
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTVarDef>(child_node);
            node->var_defs.push_back(child_node_ptr);
            s.pop();
        }

        return node;
    }
    /*
    VarDef : ID { $$ = node("VarDef", 1, $1); }
        | ID ConstArrayIdent { $$ = node("VarDef", 1, $1); }
        | ID ASSIGN InitVal { $$ = node("VarDef", 3, $1, $2, $3); }
        | ID ConstArrayIdent ASSIGN InitVal { $$ = node("VarDef", 4, $1, $2, $3, $4); }
    */
    else if(_STR_EQ(n->name,"VarDef"))
    {
        auto node = new ASTVarDef();
        node->id = n->children[0]->name;
        if(n->children_num == 2){
            auto array_ident = static_cast<ASTConstArrayIdent *>(transform_node_iter(n->children[1]));
            node->array_ident = std::shared_ptr<ASTConstArrayIdent>(array_ident);
        }
        else if(n->children_num == 3){
            auto init_val = static_cast<ASTInitVal *>(transform_node_iter(n->children[2]));
            node->init_val = std::shared_ptr<ASTInitVal>(init_val);
        }
        else if(n->children_num == 4){
            auto array_ident = static_cast<ASTConstArrayIdent *>(transform_node_iter(n->children[1]));
            node->array_ident = std::shared_ptr<ASTConstArrayIdent>(array_ident);
            auto init_val = static_cast<ASTInitVal *>(transform_node_iter(n->children[3]));
            node->init_val = std::shared_ptr<ASTInitVal>(init_val);
        }
        else{
            _AST_NODE_ERROR_
        }

        return node;
    }
    /*
    InitVal : Exp { $$ = node("InitVal", 1, $1); }
        | LBRACE InitValArrayList RBRACE { $$ = node("InitVal", 3, $1, $2, $3); }
        | LBRACE RBRACE { $$ = node("InitVal", 2, $1, $2); }
        ;

    InitValArrayList : InitValArrayList COMMA InitVal { $$ = node("InitValArrayList", 3, $1, $2, $3); }
        | InitVal { $$ = node("InitValArrayList", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name, "InitVal"))
    {
        if(n->children_num == 1)
        {
            auto exp = static_cast<ASTExp *>(transform_node_iter(n->children[0]));
            node->exp = std::shared_ptr<ASTExp>(exp);
        }
        else if(n->children_num == 3)
        {
            auto list_ptr = n->children[1];
            std::stack<syntax_tree_node *> s;
            if(list_ptr->children_num == 3)
            {
                while(list_ptr->children_num == 3)
                {
                    s.push(list_ptr->children[2]);
                    list_ptr = list_ptr->children[0];
                }
            }
            s.push(list_ptr->children[0]);  
            while(!s.empty())
            {
                auto child_node = static_cast<ASTInitVal *>(transform_node_iter(s.top()));
                if(child_node == nullptr)
                {
                    _AST_NODE_ERROR_;
                }
                auto child_node_ptr = std::shared_ptr<ASTInitVal>(child_node);
                node->init_vals.push_back(child_node_ptr);
                s.pop();
            }
        }
    
        return node;
    }
    /*
    FuncDef :   FuncType ID LPARENTHESIS FuncFParams RPARENTHESIS Block { $$ = node("FuncDef", 6, $1, $2, $3, $4, $5, $6); }
        |   FuncType ID LPARENTHESIS RPARENTHESIS Block { $$ = node("FuncDef", 5, $1, $2, $3, $4, $5); }
        ;

    FuncType :  INT { $$ = node("FuncType", 1, $1); }
        |  FLOAT { $$ = node("FuncType", 1, $1); }
        |   VOID { $$ = node("FuncType", 1, $1); }
        ;
    
    FuncFParams :   FuncFParams COMMA FuncFParam { $$ = node("FuncFParams", 3, $1, $2, $3); }
        |   FuncFParam { $$ = node("FuncFParams", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name, "FuncDef"))
    {
        auto node = new ASTFuncDef();
        if(n->children[0]->name == "FuncType")
        {
            if(n->children[0]->children[0]->name == "int"){
                node->type = TYPE_INT;
            }
            else if(n->children[0]->children[0]->name == "float"){
                node->type = TYPE_float;
            }
            else if(n->children[0]->children[0]->name == "void"){
                node->type = TYPE_VOID;
            }
        }
        else 
        {   
            _AST_NODE_ERROR_
        }
        node->id = n->children[1]->name;
        if(n->children_num == 6)
        {
            auto list_ptr = n->children[3];
            std::stack<syntax_tree_node *> s;
            if(list_ptr->children_num == 3)
            {
                while(list_ptr->children_num == 3)
                {
                    s.push(list_ptr->children[2]);
                    list_ptr = list_ptr->children[0];
                }
            }
            s.push(list_ptr->children[0]);

            while (!s.empty()) {
                auto child_node = static_cast<ASTFuncFParam *>(transform_node_iter(s.top()));
                if(child_node == nullptr){
                    _AST_NODE_ERROR_;
                }
                auto child_node_ptr = std::shared_ptr<ASTFuncFParam>(child_node);
                node->func_fparams.push_back(child_node_ptr);
                s.pop();
            }
        }
        auto block = static_cast<ASTBlock *>(transform_node_iter(n->children[n->children_num - 1]));
        node->block = std::shared_ptr<ASTBlock>(block);
    
        return node;
    }
    /*
    FuncFParam :    BType ID { $$ = node("FuncFParam", 2, $1, $2); }
        |   BType ID LBRACKET RBRACKET FuncArrayIdent { $$ = node("FuncFParam", 2, $1, $2); }
        ;
    */
    else if(_STR_EQ(n->name,"FuncFParam"))
    {
        auto node = new ASTFuncFParam();
        if(n->children[0]->name == "BType")
        {
            if(n->children[0]->children[0]->name == "int"){
                node->type = TYPE_INT;
            }
            else if(n->children[0]->children[0]->name == "float"){
                node->type = TYPE_float;
            }
            else{
                _AST_NODE_ERROR_
            }        
        }
        node->id = n->children[1]->name;
        if(n->children_num == 5)
        {
            auto array_ident =  static_cast<ASTFuncArrayIdent *>(transform_node_iter(n->children[4]));
            node->array_ident = std::shared_ptr<ASTFuncArrayIdent>(array_ident);
        }

        return node;
    }
    /*
    FuncArrayIdent :  FuncArrayIdent LBRACKET Exp RBRACKET { $$ = node("FuncArrayIdent", 4, $1, $2, $3, $4); } 
        |   empty { $$ = node("FuncArrayIdent", 0); }
        ;
    */
    else if(_STR_EQ(n->name,"FuncArrayIdent"))
    {
        auto node = new ASTFuncArrayIdent();
        auto list_ptr = n;
        std::stack<syntax_tree_node *> s;
        if(list_ptr->children_num == 4)
        {
            while(list_ptr->children_num == 4)
            {
                s.push(list_ptr->children[2]);
                list_ptr = list_ptr->children[0];
            }
        }
        
        while(!s.empty())
        {
            auto child_node = static_cast<ASTExp *>(transform_node_iter(s.top()));
            if(child_node == nullptr)
            {
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTExp>(child_node);
            node->exps.push_back(child_node_ptr);
            s.pop();
        }
        return node;
    }
    /*
    Block : LBRACE BlockItemList RBRACE { $$ = node("Block", 3, $1, $2, $3); }
        | LBRACE RBRACE { $$ = node("Block", 2, $1, $2); }
        ;

    BlockItemList : BlockItemList BlockItem { $$ = node("BlockItemList", 2, $1, $2); }
        | BlockItem { $$ = node("BlockItemList", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"Block"))
    {
        auto node = new ASTBlock();
        if(n->children_num == 3)
        {
            auto list_ptr = n->children[1];
            std::stack<syntax_tree_node *> s;
            if(list_ptr->children_num == 2)
            {
                while(list_ptr->children_num == 2)
                {
                    s.push(list_ptr->children[1]);
                    list_ptr = list_ptr->children[0];
                }
            }
            s.push(list_ptr->children[0]);

            while (!s.empty()) {
                auto child_node = static_cast<ASTBlockItem *>(transform_node_iter(s.top()));
                if(child_node == nullptr){
                    _AST_NODE_ERROR_;
                }
                auto child_node_ptr = std::shared_ptr<ASTBlockItem>(child_node);
                node->block_items.push_back(child_node_ptr);
                s.pop();
            }
        }
        return node;
    }
    /*
    BlockItem : Stmt { $$ = node("BlockItem", 1, $1); }
        | ConstDecl { $$ = node("BlockItem", 1, $1); }
        | VarDecl { $$ = node("BlockItem", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"BlockItem"))
    {
        auto node = new ASTBlockItem();
        if(n->children[0]->name == "Stmt")
        {
            auto stmt = static_cast<ASTStmt *>(transform_node_iter(n->children[0]));
            node->stmt = std::shared_ptr<ASTStmt>(stmt);
        }
        else if(n->children[0]->name == "ConstDecl")
        {
            auto const_decl = static_cast<ASTConstDecl *>(transform_node_iter(n->children[0]));
            node->const_decl = std::shared_ptr<ASTConstDecl>(const_decl);
        }
        else if(n->children[0]->name == "VarDecl")
        {
            auto var_decl = static_cast<ASTVarDecl *>(transform_node_iter(n->children[0]));
            node->var_decl = std::shared_ptr<ASTVarDecl>(var_decl);
        }
        else{
            _AST_NODE_ERROR_
        }
        return node;
    }
    /*
    Stmt : LVal ASSIGN Exp SEMICOLON { $$ = node("Stmt", 4, $1, $2, $3, $4); }
        | Exp SEMICOLON { $$ = node("Stmt", 2, $1, $2); }
        | Block { $$ = node("Stmt", 1, $1); }
        | IF LPARENTHESIS Cond RPARENTHESIS Stmt { $$ = node("Stmt", 5, $1, $2, $3, $4, $5); }
        | IF LPARENTHESIS Cond RPARENTHESIS Stmt ELSE Stmt { $$ = node("Stmt", 7, $1, $2, $3, $4, $5, $6, $7); }
        | WHILE LPARENTHESIS Cond RPARENTHESIS Stmt { $$ = node("Stmt", 5, $1, $2, $3, $4, $5); }
        | RETURN SEMICOLON { $$ = node("Stmt", 2, $1, $2); }
        | RETURN Exp SEMICOLON { $$ = node("Stmt", 3, $1, $2, $3); }
        | BREAK SEMICOLON { $$ = node("Stmt", 2, $1, $2); }
        | CONTINUE SEMICOLON { $$ = node("Stmt", 2, $1, $2); }
        ;    
    */
    else if(_STR_EQ(n->name,"Stmt"))
    {
        auto node = new ASTStmt();
        if(n->children[0]->name == "LVal")
        {
            auto lval = static_cast<ASTLVal *>(transform_node_iter(n->children[0]));
            node->lval = std::shared_ptr<ASTLVal>(lval);
            auto exp = static_cast<ASTExp *>(transform_node_iter(n->children[2]));
            node->exp = std::shared_ptr<ASTExp>(exp);
        }
        else if(n->children[0]->name == "Exp")
        {
            auto exp = static_cast<ASTExp *>(transform_node_iter(n->children[0]));
            node->exp = std::shared_ptr<ASTExp>(exp);
        }
        else if(n->children[0]->name == "Block")
        {
            auto block = static_cast<ASTBlock *>(transform_node_iter(n->children[0]));
            node->block = std::shared_ptr<ASTBlock>(block);
        }
        else if(n->children[0]->name == "IF")
        {
            auto selection_stmt = new ASTSelectionStmt();
            auto cond = static_cast<ASTCond *>(transform_node_iter(n->children[2]));
            selection_stmt->cond = std::shared_ptr<ASTCond>(cond);
            auto stmt1 = static_cast<ASTStmt *>(transform_node_iter(n->children[4]));
            selection_stmt->if_stmt = std::shared_ptr<ASTStmt>(stmt1);
            if(n->children_num == 7)
            {
                auto stmt2 = static_cast<ASTStmt *>(transform_node_iter(n->children[6]));
                selection_stmt->else_stmt = std::shared_ptr<ASTStmt>(stmt2);
            }
            node->selection_stmt = std::shared_ptr<ASTSelectionStmt>(selection_stmt);
        }
        else if(n->children[0]->name == "WHILE")
        {
            auto iteration_stmt = new ASTIterationStmt();
            auto cond = static_cast<ASTCond *>(transform_node_iter(n->children[2]));
            iteration_stmt->cond = std::shared_ptr<ASTCond>(cond);
            auto stmt = static_cast<ASTStmt *>(transform_node_iter(n->children[4]));
            iteration_stmt->stmt = std::shared_ptr<ASTStmt>(stmt);
            node->iteration_stmt = std::shared_ptr<ASTIterationStmt>(iteration_stmt);
        }
        else if(n->children[0]->name == "RETURN")
        {
            auto return_stmt = new ASTReturnStmt();
            if(n->children_num == 3)
            {
                auto exp = static_cast<ASTExp *>(transform_node_iter(n->children[1]));
                return_stmt->exp = std::shared_ptr<ASTExp>(exp);
            }
            node->return_stmt = std::shared_ptr<ASTReturnStmt>(return_stmt);
        }
        else if(n->children[0]->name == "BREAK")
        {
            auto break_stmt = new ASTBreakStmt();
            node->break_stmt = std::shared_ptr<ASTBreakStmt>(break_stmt);
        }
        else if(n->children[0]->name == "CONTINUE")
        {
            auto continue_stmt = new ASTContinueStmt();
            node->continue_stmt = std::shared_ptr<ASTContinueStmt>(continue_stmt);
        }
        else{
            _AST_NODE_ERROR_
        }
        
        return node;
    }
    /*
    Exp : AddExp { $$ = node("Exp", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"Exp"))
    {
        auto node = new ASTExp();
        auto add_exp = static_cast<ASTAddExp *>(transform_node_iter(n->children[0]));
        node->add_exp = std::shared_ptr<ASTAddExp>(add_exp);

        return node;
    }
    /*
    Cond : LOrExp { $$ = node("Cond", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"Cond"))
    {
        auto node = new ASTCond();
        auto l_or_exp = static_cast<ASTLOrExp *>(transform_node_iter(n->children[0]));
        node->l_or_exp = std::shared_ptr<ASTLOrExp>(l_or_exp); 

        return node;
    }
    /*
    LVal : ID { $$ = node("LVal", 1, $1); }
        |  ID ArrayIdent { $$ = node("LVal", 2, $1, $2); }
        ;
    */
    else if(_STR_EQ(n->name,"LVal"))
    {
        auto node = new ASTLVal();
        auto node->id = n->children[0]->name;
        if(n->children_num == 2)
        {
            auto array_ident = static_cast<ASTArrayIdent *>(transform_node_iter(n->children[1]));
            node->array_ident = std::shared_ptr<ASTArrayIdent>(array_ident);
        }

        return node;
    }
    /*
    ArrayIdent: ArrayIdent LBRACKET Exp RBRACKET { $$ = node("ArrayIdent", 4, $1, $2, $3, $4); }
        |empty { $$ = node("ArrayIdent", 0); }
        ;
    */
    else if(_STR_EQ(n->name,"ArrayIdent"))
    {
        auto node = new ASTArrayIdent();
        auto list_ptr = n;
        std::stack<syntax_tree_node *> s;
        if(list_ptr->children_num == 4)
        {
            while(list_ptr->children_num == 4)
            {
                s.push(list_ptr->children[2]);
                list_ptr = list_ptr->children[0];
            }
        }
        
        while(!s.empty())
        {
            auto child_node = static_cast<ASTExp *>(transform_node_iter(s.top()));
            if(child_node == nullptr)
            {
                _AST_NODE_ERROR_;
            }
            auto child_node_ptr = std::shared_ptr<ASTExp>(child_node);
            node->exps.push_back(child_node_ptr);
            s.pop();
        }
        return node;
    }
    /*
    PrimaryExp : LPARENTHESIS Exp RPARENTHESIS { $$ = node("PrimaryExp", 3, $1, $2, $3); }
        | LVal { $$ = node("PrimaryExp", 1, $1); }
        | Number { $$ = node("PrimaryExp", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"PrimaryExp"))
    {
        auto node = new ASTPrimaryExp();
        if(n->children_num == 3)
        {
            auto exp = static_cast<ASTExp *>(transform_node_iter(n->children[1]));
            node->exp = std::shared_ptr<ASTExp>(exp);
        }
        else if(n->children[0]->name == "LVal")
        {
            auto lval = static_cast<ASTLVal *>(transform_node_iter(n->children[0]));
            node->lval = std::shared_ptr<ASTLVal>(lval);
        }
        else if(n->children[0]->name == "Number")
        {
            auto number = static_cast<ASTNumber *>(transform_node_iter(n->children[0]));
            node->number = std::shared_ptr<ASTNumber>(number);
        }
        else{
            _AST_NODE_ERROR_
        }

        return node;
    }
    /*
    Number : INTEGER_10 { $$ = node("Number", 1, $1); }
        | INTEGER_8 { $$ = node("Number", 1, $1); }
        | INTEGER_16 { $$ = node("Number", 1, $1); }
        | FLOATPOINT_10 { $$ = node("Number", 1, $1); }
        | FLOATPOINT_16 { $$ = node("Number", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"Number"))
    {
        auto node = new ASTNumber();
        if(n->children[0]->name == "Integer")
        {
            node->type = TYPE_INT;
            node->int_val = std::stoi(n->children[0]->children[0]->name);
        }
        else if(n->children[0]->name == "FloatPoint")
        {
            node->type = TYPE_FLOAT;
            node->float_val = std::stof(n->children[0]->children[0]->name);
        }
        return node;
    }
    /*
    UnaryExp : PrimaryExp { $$ = node("UnaryExp", 1, $1); }
        | UnaryOp UnaryExp { $$ = node("UnaryExp", 2, $1, $2); }
        | ID LPARENTHESIS FuncRParams RPARENTHESIS { $$ = node("UnaryExp", 4, $1, $2, $3, $4); }
        | ID LPARENTHESIS RPARENTHESIS { $$ = node("UnaryExp", 3, $1, $2, $3); }
        ;
    
    FuncRParams : FuncRParams COMMA Exp { $$ = node("FuncRParams", 3, $1, $2, $3); }
        | Exp { $$ = node("FuncRParams", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"UnaryExp"))
    {
        auto node = new ASTUnaryExp();
        if(n->children[0]->name == "PrimaryExp")
        {
            auto primary_exp = static_cast<ASTPrimaryExp *>(transform_node_iter(n->children[0]));
            node->primary_exp = std::shared_ptr<ASTPrimaryExp>(primary_exp);
        }
        else if(n->children[0]->name == "UnaryOp")
        {
            if(n->children[0]->children[0]->name == "MINUS")
            {
                node->op = OP_MINUS;
            }
            else if(n->children[0]->children[0]->name == "NOT")
            {
                node->op = OP_NOT;
            }
            else if(n->children[0]->children[0]->name == "PLUS")
            {
                node->op = OP_PLUS;
            }
            else{
                _AST_NODE_ERROR_
            }
            auto unary_exp = static_cast<ASTUnaryExp *>(transform_node_iter(n->children[1]));
            node->unary_exp = std::shared_ptr<ASTUnaryExp>(unary_exp);
        }
        else 
        {
            auto call = new ASTCall();
            call->id = n->children[0]->name;
            if(n->children_num == 4)
            {
                auto list_ptr = n->children[2];
                std::stack<syntax_tree_node *> s;
                if(list_ptr->children_num == 3)
                {
                    while(list_ptr->children_num == 3)
                    {
                        s.push(list_ptr->children[2]);
                        list_ptr = list_ptr->children[0];
                    }
                }
                s.push(list_ptr->children[0]);

                while (!s.empty()) {
                    auto child_node = static_cast<ASTExp *>(transform_node_iter(s.top()));
                    if(child_node == nullptr){
                        _AST_NODE_ERROR_;
                    }
                    auto child_node_ptr = std::shared_ptr<ASTExp>(child_node);
                    call->exps.push_back(child_node_ptr);
                    s.pop();
                }
            }
            node->call = std::shared_ptr<ASTCall>(call);
        }

        return node;
    }
    /*
    MulExp : UnaryExp { $$ = node("MulExp", 1, $1); }
        | MulExp MUL UnaryExp { $$ = node("MulExp", 3, $1, $2, $3); }
        | MulExp DIV UnaryExp { $$ = node("MulExp", 3, $1, $2, $3); }
        | MulExp MOD UnaryExp { $$ = node("MulExp", 3, $1, $2, $3); }
        ;
    */
    else if(_STR_EQ(n->name,"MulExp"))
    {
        if(n->children_num == 1)
        {
            auto unary_exp = static_cast<ASTUnaryExp *>(transform_node_iter(n->children[0]));
            node->unary_exp = std::shared_ptr<ASTUnaryExp>(unary_exp);
        }
        else if(n->children_num == 3)
        {
            auto mul_exp = static_cast<ASTMulExp *>(transform_node_iter(n->children[0]));
            node->mul_exp = std::shared_ptr<ASTMulExp>(mul_exp);
            auto unary_exp = static_cast<ASTUnaryExp *>(transform_node_iter(n->children[2]));
            node->unary_exp = std::shared_ptr<ASTUnaryExp>(unary_exp);
            if(n->children[1]->name == "MUL")
            {
                node->op = OP_MUL;
            }
            else if(n->children[1]->name == "DIV")
            {
                node->op = OP_DIV;
            }
            else if(n->children[1]->name == "MOD")
            {
                node->op = OP_MOD;
            }
            else{
                _AST_NODE_ERROR_
            }
        }
        else{
            _AST_NODE_ERROR_
        }
        return node;
    }
    /*
    AddExp : MulExp { $$ = node("AddExp", 1, $1); }
        | AddExp ADD MulExp { $$ = node("AddExp", 3, $1, $2, $3); }
        | AddExp SUB MulExp { $$ = node("AddExp", 3, $1, $2, $3); }
        ;
    */
    else if(_STR_EQ(n->name,"AddExp"))
    {
        if(n->children_num == 1)
        {
            auto mul_exp = static_cast<ASTMulExp *>(transform_node_iter(n->children[0]));
            node->mul_exp = std::shared_ptr<ASTMulExp>(mul_exp);
        }
        else if(n->children_num == 3)
        {
            auto add_exp = static_cast<ASTAddExp *>(transform_node_iter(n->children[0]));
            node->add_exp = std::shared_ptr<ASTAddExp>(add_exp);
            auto mul_exp = static_cast<ASTMulExp *>(transform_node_iter(n->children[2]));
            node->mul_exp = std::shared_ptr<ASTMulExp>(mul_exp);
            if(n->children[1]->name == "ADD")
            {
                node->op = OP_ADD;
            }
            else if(n->children[1]->name == "SUB")
            {
                node->op = OP_SUB;
            }
            else{
                _AST_NODE_ERROR_
            }
        }
        else{
            _AST_NODE_ERROR_
        }
        return node;
    }
    /*
    LOrExp : LAndExp { $$ = node("LOrExp", 1, $1); }
        | LOrExp OR LAndExp { $$ = node("LOrExp", 3, $1, $2, $3); }
        ;
    */
    else if(_STR_EQ(n->name,"LOrExp"))
    {
        if(n->children_num == 1)
        {
            auto l_and_exp = static_cast<ASTLAndExp *>(transform_node_iter(n->children[0]));
            node->l_and_exp = std::shared_ptr<ASTLAndExp>(l_and_exp);
        }
        else if(n->children_num == 3)
        {
            auto l_or_exp = static_cast<ASTLOrExp *>(transform_node_iter(n->children[0]));
            node->l_or_exp = std::shared_ptr<ASTLOrExp>(l_or_exp);
            auto l_and_exp = static_cast<ASTLAndExp *>(transform_node_iter(n->children[2]));
            node->l_and_exp = std::shared_ptr<ASTLAndExp>(l_and_exp);
            node->op = OP_OR;
        }
        else{
            _AST_NODE_ERROR_
        }
        return node;
    }
    /*
    LAndExp : EqExp { $$ = node("LAndExp", 1, $1); }
        | LAndExp AND EqExp { $$ = node("LAndExp", 3, $1, $2, $3); }
        ;
    */
    else if(_STR_EQ(n->name,"LAndExp"))
    {
        if(n->children_num == 1)
        {
            auto eq_exp = static_cast<ASTEqExp *>(transform_node_iter(n->children[0]));
            node->eq_exp = std::shared_ptr<ASTEqExp>(eq_exp);
        }
        else if(n->children_num == 3)
        {
            auto l_and_exp = static_cast<ASTLAndExp *>(transform_node_iter(n->children[0]));
            node->l_and_exp = std::shared_ptr<ASTLAndExp>(l_and_exp);
            auto eq_exp = static_cast<ASTEqExp *>(transform_node_iter(n->children[2]));
            node->eq_exp = std::shared_ptr<ASTEqExp>(eq_exp);
            node->op = OP_AND;
        }
        else{
            _AST_NODE_ERROR_
        }
    }
    /*
    RelExp : AddExp { $$ = node("RelExp", 1, $1); }
        | RelExp LT AddExp { $$ = node("RelExp", 3, $1, $2, $3); }
        | RelExp GT AddExp { $$ = node("RelExp", 3, $1, $2, $3); }
        | RelExp LE AddExp { $$ = node("RelExp", 3, $1, $2, $3); }
        | RelExp GE AddExp { $$ = node("RelExp", 3, $1, $2, $3); }
        ;
    */
    else if(_STR_EQ(n->name,"RelExp"))
    {
        if(n->children_num == 1)
        {
            auto add_exp = static_cast<ASTAddExp *>(transform_node_iter(n->children[0]));
            node->add_exp = std::shared_ptr<ASTAddExp>(add_exp);
        }
        else if(n->children_num == 3)
        {
            auto rel_exp = static_cast<ASTRelExp *>(transform_node_iter(n->children[0]));
            node->rel_exp = std::shared_ptr<ASTRelExp>(rel_exp);
            auto add_exp = static_cast<ASTAddExp *>(transform_node_iter(n->children[2]));
            node->add_exp = std::shared_ptr<ASTAddExp>(add_exp);
            if(n->children[1]->name == "LT")
            {
                node->op = OP_LT;
            }
            else if(n->children[1]->name == "GT")
            {
                node->op = OP_GT;
            }
            else if(n->children[1]->name == "LE")
            {
                node->op = OP_LE;
            }
            else if(n->children[1]->name == "GE")
            {
                node->op = OP_GE;
            }
            else{
                _AST_NODE_ERROR_
            }
        }
        else{
            _AST_NODE_ERROR_
        }
        return node;
    }
    /*
    ConstExp : AddExp { $$ = node("ConstExp", 1, $1); }
        ;
    */
    else if(_STR_EQ(n->name,"ConstExp"))
    {
        auto node = new ASTConstExp();
        auto add_exp = static_cast<ASTAddExp *>(transform_node_iter(n->children[0]));
        node->add_exp = std::shared_ptr<ASTAddExp>(add_exp);

        return node;
    }
    else{
        _AST_NODE_ERROR_
    }
    else {
        std::cerr << "[ast]: transform failure!" << std::endl;
        std::abort();
    }
}

Value *ASTCompUnit::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTDecl::accept(Visitor *visitor) {
    if(this->const_decl != nullptr){
        return this->const_decl->accept(visitor);
    }
    else if(this->var_decl != nullptr){
        return this->var_decl->accept(visitor);
    }
    else if(this->func_def != nullptr){
        return this->func_def->accept(visitor);
    }
    else{
        std::cerr << "[ast]: accept failure!" << std::endl;
        std::abort();
    }
}
Value *ASTConstDecl::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTConstDef::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTConstInitVal::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTVarDecl::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTVarDef::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTInitVal::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTFuncDef::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTFuncFParam::accept(Visitor *visitor) {return visitor->visit(this);}
//Value *ASTFuncArrayIdent::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTBlock::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTBlockItem::accept(Visitor *visitor) {
    if(this->stmt != nullptr){
        return this->stmt->accept(visitor);
    }
    else if(this->const_decl != nullptr){
        return this->const_decl->accept(visitor);
    }
    else if(this->var_decl != nullptr){
        return this->var_decl->accept(visitor);
    }
    else{
        std::cerr << "[ast]: accept failure!" << std::endl;
        std::abort();
    }
}
Value *ASTStmt::accept(Visitor *visitor) {
    if(this->lval != nullptr){
        return this->lval->accept(visitor);
    }
    else if(this->exp != nullptr){
        return this->exp->accept(visitor);
    }
    else if(this->block != nullptr){
        return this->block->accept(visitor);
    }
    else if(this->selection_stmt != nullptr){
        return this->selection_stmt->accept(visitor);
    }
    else if(this->iteration_stmt != nullptr){
        return this->iteration_stmt->accept(visitor);
    }
    else if(this->return_stmt != nullptr){
        return this->return_stmt->accept(visitor);
    }
    else if(this->break_stmt != nullptr){
        return this->break_stmt->accept(visitor);
    }
    else if(this->continue_stmt != nullptr){
        return this->continue_stmt->accept(visitor);
    }
    else{
        std::cerr << "[ast]: accept failure!" << std::endl;
        std::abort();
    }
}
Value *ASTExp::accept(Visitor *visitor) {return this->add_exp->accept(visitor);}
Value *ASTCond::accept(Visitor *visitor) {return this->l_or_exp->accept(visitor);}
Value *ASTLVal::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTPrimaryExp::accept(Visitor *visitor) {visitor->visit(this);}
Value *ASTNumber::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTUnaryExp::accept(Visitor *visitor) { return visitor->visit(this);}
Value *ASTMulExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTAddExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTLOrExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTLAndExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTRelExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTConstExp::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTSelectionStmt::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTIterationStmt::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTReturnStmt::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTBreakStmt::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTContinueStmt::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTCall::accept(Visitor *visitor) {return visitor->visit(this);}
Value *ASTEqExp::accept(Visitor *visitor) {return visitor->visit(this);}