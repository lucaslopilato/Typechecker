#include <iostream>
#include <cstdio>
#include <cstring>

#include "ast.hpp"
#include "symtab.hpp"
#include "primitive.hpp"
#include "assert.h"

// WRITEME: The default attribute propagation rule
#define default_rule(X) (X->visit_children(this))

#include <typeinfo>

class Typecheck : public Visitor
{
  private:
    FILE* m_errorfile;
    SymTab* m_st;

    // The set of recognized errors
    enum errortype
    {
        no_main,
        nonvoid_main,
        dup_proc_name,
        dup_var_name,
        proc_undef,
        call_type_mismatch,
        narg_mismatch,
        expr_type_err,
        var_undef,
        ifpred_err,
        whilepred_err,
        incompat_assign,
        who_knows,
        ret_type_mismatch,
        array_index_error,
        no_array_var,
        arg_type_mismatch,
        expr_pointer_arithmetic_err,
        expr_abs_error,
        expr_addressof_error,
        invalid_deref
    };

    // Print the error to file and exit
    void t_error(errortype e, Attribute a)
    {
        fprintf(m_errorfile,"on line number %d, ", a.lineno);

        switch(e)
        {
            case no_main:
                fprintf(m_errorfile, "error: no main\n");
                exit(2);
            case nonvoid_main:
                fprintf(m_errorfile, "error: the Main procedure has arguments\n");
                exit(3);
            case dup_proc_name:
                fprintf(m_errorfile, "error: duplicate procedure names in same scope\n");
                exit(4);
            case dup_var_name:
                fprintf(m_errorfile, "error: duplicate variable names in same scope\n");
                exit(5);
            case proc_undef:
                fprintf(m_errorfile, "error: call to undefined procedure\n");
                exit(6);
            case var_undef:
                fprintf(m_errorfile, "error: undefined variable\n");
                exit(7);
            case narg_mismatch:
                fprintf(m_errorfile, "error: procedure call has different number of args than declartion\n");
                exit(8);
            case arg_type_mismatch:
                fprintf(m_errorfile, "error: argument type mismatch\n");
                exit(9);
            case ret_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in return statement\n");
                exit(10);
            case call_type_mismatch:
                fprintf(m_errorfile, "error: type mismatch in procedure call args\n");
                exit(11);
            case ifpred_err:
                fprintf(m_errorfile, "error: predicate of if statement is not boolean\n");
                exit(12);
            case whilepred_err:
                fprintf(m_errorfile, "error: predicate of while statement is not boolean\n");
                exit(13);
            case array_index_error:
                fprintf(m_errorfile, "error: array index not integer\n");
                exit(14);
            case no_array_var:
                fprintf(m_errorfile, "error: attempt to index non-array variable\n");
                exit(15);
            case incompat_assign:
                fprintf(m_errorfile, "error: type of expr and var do not match in assignment\n");
                exit(16);
            case expr_type_err:
                fprintf(m_errorfile, "error: incompatible types used in expression\n");
                exit(17);
            case expr_abs_error:
                fprintf(m_errorfile, "error: absolute value can only be applied to integers and strings\n");
                exit(17);
            case expr_pointer_arithmetic_err:
                fprintf(m_errorfile, "error: invalid pointer arithmetic\n");
                exit(18);
            case expr_addressof_error:
                fprintf(m_errorfile, "error: AddressOf can only be applied to integers, chars, and indexed strings\n");
                exit(19);
            case invalid_deref:
                fprintf(m_errorfile, "error: Deref can only be applied to integer pointers and char pointers\n");
                exit(20);
            default:
                fprintf(m_errorfile, "error: no good reason\n");
                exit(21);
        }
    }

    // Helpers
    // WRITEME: You might want write some hepler functions.

    const char * lhs_to_id (Lhs* lhs){
        Variable *v = dynamic_cast<Variable*>(lhs);
        if(v) {
                return strdup(v->m_symname->spelling());
        }

        DerefVariable *dv = dynamic_cast<DerefVariable*>(lhs);
        if(dv) {
            return dv->m_symname->spelling();
        }   

        ArrayElement *ae = dynamic_cast<ArrayElement*>(lhs);
        if(ae) {
            return ae->m_symname->spelling();
        }

        return nullptr;
    }
    // Type Checking
    // WRITEME: You need to implement type-checking for this project

    // Check that there is one and only one main
    void check_for_one_main(ProgramImpl* p)
    {
        if(!m_st->exist(strdup("Main"))){
            this->t_error(no_main, p->m_attribute);
        }

        Symbol* main = m_st->lookup("Main");
        if(!main->m_arg_type.empty()){
            this->t_error(nonvoid_main, p->m_attribute);
        }
    }

    // Create a symbol for the procedure and check there is none already
    // existing
    void add_proc_symbol(ProcImpl* p)
    {
        char* name;
        Symbol* s;
        
        //Initialize Base Symbol Attributes
        s = new Symbol();
        name = strdup(p->m_symname->spelling());
        s->m_basetype = bt_procedure;

        //this->m_st->dump(stdout);
        //Check to see if procedure already exists at the same scope
        //SymScope* currentScope = this->m_st->get_scope();
        //Symbol* exists = this->m_st->lookup(currentScope, name);
        //if(exists != NULL){
        //    this->t_error(dup_proc_name, p->m_attribute);
        //}

        //Initialize Procedure Attributes
        s->m_return_type = p->m_type->m_attribute.m_basetype;
        s->m_arg_type = std::vector<Basetype>();
        
        //For Visit Each Declaration
        for(std::list<Decl_ptr>::iterator iter = p->m_decl_list->begin();
            iter != p->m_decl_list->end(); ++iter)
        {
             DeclImpl* current = dynamic_cast<DeclImpl*>(*iter);
             //Push number of types per variable declared
             if(current)
             for(int i=0; i<(*current).m_symname_list->size(); i++){
                s->m_arg_type.push_back((*iter)->m_attribute.m_basetype);
             }
        }
        if(!m_st->insert(name, s)){ //Check if symbol is not already present
                this->t_error(dup_proc_name, p->m_attribute);
        }

        //Symbol* check = m_st
    }

    // Add symbol table information for all the declarations following
    void add_decl_symbol(DeclImpl* p)
    {
        char* name;
        Symbol* s;
        for(std::list<SymName_ptr>::iterator iter = p->m_symname_list->begin();
            iter != p->m_symname_list->end(); ++iter)
        {
            name = strdup((*iter)->spelling());
            s = new Symbol();
            s->m_basetype = p->m_type->m_attribute.m_basetype;

            if(!m_st->insert(name, s)){ //Check if symbol is not already present
                this->t_error(dup_var_name, p->m_attribute);
            }    
            
        }
    }

    // Check that the return statement of a procedure has the appropriate type
    void check_proc(ProcImpl *p)
    {
        //Type m_type
        //if(!
    }
    
    //TODO Pretty sure do not need
    // Check that the declared return type is not an array
    void check_return(Return *p)
    {
        if(p->m_expr->m_attribute.m_basetype == bt_string){
            this->t_error(ret_type_mismatch, p->m_attribute);
        }
    }

    // Check that function being called exists and that types of arguments
    // and return values are consistent
    void check_call(Call *p)
    {
        char* pName = strdup(p->m_symname->spelling());
        //Check if the procedure is defined
        if(!this->m_st->exist(pName)){
            this->t_error(proc_undef, p->m_attribute);
        }
        //Check if the lhs is defined
        else if(!this->m_st->exist(strdup(lhs_to_id(p->m_lhs)))){
            this->t_error(var_undef, p->m_attribute);
        }
        else{
            //Lookup the symbol to reference
            Symbol * s = this->m_st->lookup(pName);

            //Make sure number of arguments provided matches symbol
            if(s->m_arg_type.size() != p->m_expr_list->size()){
                this->t_error(narg_mismatch, p->m_attribute);
            }

            //Run through each type and make sure they are the same
            std::vector<Basetype>::iterator sym = s->m_arg_type.begin();

           for(std::list<Expr_ptr>::iterator iter = p->m_expr_list->begin();
            iter != p->m_expr_list->end(); ++iter)
            {
                //Compare BaseTypes of basetype to Expression
                if((*iter)->m_attribute.m_basetype != (*sym)){
                    this->t_error(arg_type_mismatch, p->m_attribute);
                }
                //Advance Symbol Arguments
                sym++;
            }

            
        }
    }

    // For checking that this expressions type is boolean used in if/else
    void check_pred_if(Expr* p)
    {
        if(p->m_attribute.m_basetype != bt_boolean){
            this->t_error(ifpred_err, p->m_attribute);
        }
    }

    // For checking that this expressions type is boolean used in while
    void check_pred_while(Expr* p)
    {
    }

    void check_assignment(Assignment* p)
    {
        
    }

    void check_string_assignment(StringAssignment* p)
    {
        if(!m_st->exist(strdup(lhs_to_id(p->m_lhs)))){
            this->t_error(no_array_var, p->m_attribute);
        }
    }

    void check_array_access(ArrayAccess* p)
    {
    }

    void check_array_element(ArrayElement* p)
    {
    }

    // For checking boolean operations(and, or ...)
    void checkset_boolexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking arithmetic expressions(plus, times, ...)
    void checkset_arithexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // Called by plus and minus: in these cases we allow pointer arithmetics
    void checkset_arithexpr_or_pointer(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking relational(less than , greater than, ...)
    void checkset_relationalexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking equality ops(equal, not equal)
    void checkset_equalityexpr(Expr* parent, Expr* child1, Expr* child2)
    {
    }

    // For checking not
    void checkset_not(Expr* parent, Expr* child)
    {
    }

    // For checking unary minus
    void checkset_uminus(Expr* parent, Expr* child)
    {
    }

    void checkset_absolute_value(Expr* parent, Expr* child)
    {
    }

    void checkset_addressof(Expr* parent, Lhs* child)
    {
    }

    void checkset_deref_expr(Deref* parent,Expr* child)
    {
    }

    // Check that if the right-hand side is an lhs, such as in case of
    // addressof
    void checkset_deref_lhs(DerefVariable* p)
    {
    }

    void checkset_variable(Variable* p)
    {
        if(!m_st->exist(strdup(p->m_symname->spelling())))
            this->t_error(var_undef, p->m_attribute);
    }


  public:

    Typecheck(FILE* errorfile, SymTab* st) {
        m_errorfile = errorfile;
        m_st = st;
    }

    void visitProgramImpl(ProgramImpl* p)
    {
       default_rule(p);       
       check_for_one_main(p); 
    }

    void visitProcImpl(ProcImpl* p)
    {
        //Visit Arguments to define types for the symbol
       for(std::list<Decl_ptr>::iterator iter = p->m_decl_list->begin(); 
        iter != p->m_decl_list->end(); ++iter)
        {
            (*iter)->accept(this);
        }

       //Make sure the procedure properly defined 
       check_proc(p); 
        
       //Add the new procedure symbols to the symtab
       add_proc_symbol(p); 

       //Call accept on all children besides the arguments 
       p->m_symname->accept(this);
       p->m_type->accept(this);
       p->m_procedure_block->accept(this);


    }

    void visitCall(Call* p)
    {
       default_rule(p);
       check_call(p);       
    }

    void visitNested_blockImpl(Nested_blockImpl* p)
    {
       m_st->open_scope();
       default_rule(p);   
       m_st->close_scope();    
    }

    void visitProcedure_blockImpl(Procedure_blockImpl* p)
    {
       m_st->open_scope();
       default_rule(p);  
       m_st->close_scope();     
    }

    void visitDeclImpl(DeclImpl* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = p->m_type->m_attribute.m_basetype;
       add_decl_symbol(p);
    
               
    }

    void visitAssignment(Assignment* p)
    {
       default_rule(p);
       check_assignment(p);       
    }

    void visitStringAssignment(StringAssignment *p)
    {
       default_rule(p);
       check_string_assignment(p);
       p->m_lhs->m_attribute.m_basetype = bt_string;
    }

    void visitIdent(Ident* p)
    {
       default_rule(p);
       Symbol* var = this->m_st->lookup(strdup(p->m_symname->spelling()));     
       p->m_attribute.m_basetype =  var->m_basetype; 
    }

    void visitReturn(Return* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = p->m_expr->m_attribute.m_basetype;
       check_return(p);       
    }

    void visitIfNoElse(IfNoElse* p)
    {
       default_rule(p);
       check_pred_if(p->m_expr); //Check if this is the proper func to call
    }

    void visitIfWithElse(IfWithElse* p)
    {
       default_rule(p);     
       check_pred_if(p->m_expr);  
    }

    void visitWhileLoop(WhileLoop* p)
    {
       default_rule(p);
       check_pred_while(p->m_expr);       
    }

    void visitCodeBlock(CodeBlock *p) 
    {
       m_st->open_scope();
       default_rule(p);   
       m_st->close_scope(); 
    }

    void visitTInteger(TInteger* p)
    {
       default_rule(p);      
       p->m_attribute.m_basetype = bt_integer; 
    }

    void visitTBoolean(TBoolean* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_boolean;
    }

    void visitTCharacter(TCharacter* p)
    {
       default_rule(p);   
       p->m_attribute.m_basetype = bt_char;   
    }

    void visitTString(TString* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_string;
    }

    void visitTCharPtr(TCharPtr* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_charptr; 
    }

    void visitTIntPtr(TIntPtr* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_intptr; 
    }

    void visitAnd(And* p)
    {
       default_rule(p);
       checkset_boolexpr(p, p->m_expr_1, p->m_expr_2);       
    }

    void visitDiv(Div* p)
    {
       default_rule(p);
       checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
    }

    //TODO Figure out what this Compare is
    void visitCompare(Compare* p)
    {
       default_rule(p);      
 
    }

    void visitGt(Gt* p)
    {
       default_rule(p);       
       checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitGteq(Gteq* p)
    {
       default_rule(p);       
       checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitLt(Lt* p)
    {
       default_rule(p);       
       checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitLteq(Lteq* p)
    {
       default_rule(p);       
       checkset_relationalexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitMinus(Minus* p)
    {
       default_rule(p);       
       checkset_arithexpr_or_pointer(p, p->m_expr_1, p->m_expr_2);
    }

    void visitNoteq(Noteq* p)
    {
       default_rule(p);       
       checkset_equalityexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitOr(Or* p)
    {
       default_rule(p);
       checkset_boolexpr(p, p->m_expr_1, p->m_expr_2);      
    }

    void visitPlus(Plus* p)
    {
       default_rule(p);       
       checkset_arithexpr_or_pointer(p, p->m_expr_1, p->m_expr_2);
    }

    void visitTimes(Times* p)
    {
       default_rule(p);       
       checkset_arithexpr(p, p->m_expr_1, p->m_expr_2);
    }

    void visitNot(Not* p)
    {
       default_rule(p);       
       checkset_not(p, p->m_expr);       
    }

    void visitUminus(Uminus* p)
    {
       default_rule(p);       
       checkset_uminus(p, p->m_expr);       
    }

    void visitArrayAccess(ArrayAccess* p)
    {
       default_rule(p);
       check_array_access(p);
    }

    void visitIntLit(IntLit* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_integer;       
    }

    void visitCharLit(CharLit* p)
    {
       default_rule(p);
       p->m_attribute.m_basetype = bt_char;        
    }

    void visitBoolLit(BoolLit* p)
    {
       default_rule(p);       
       p->m_attribute.m_basetype = bt_boolean;       
    }

    void visitNullLit(NullLit* p)
    {
       default_rule(p);       
    }

    void visitAbsoluteValue(AbsoluteValue* p)
    {
       default_rule(p);       
       checkset_absolute_value(p, p->m_expr);       
    }

    void visitAddressOf(AddressOf* p)
    {
       default_rule(p);       
       checkset_addressof(p, p->m_lhs);       
    }

    void visitVariable(Variable* p)
    {
       default_rule(p);   
       checkset_variable(p);
        //TODO might not need here, mixed up variable with Ident
       Symbol* var = this->m_st->lookup(strdup(p->m_symname->spelling()));     
       p->m_attribute.m_basetype =  var->m_basetype; 
    }

    void visitDeref(Deref* p)
    {
       default_rule(p);       
       checkset_deref_expr(p, p->m_expr);       
    }

    void visitDerefVariable(DerefVariable* p)
    {
       default_rule(p);       
       checkset_deref_lhs(p);       
    }

    void visitArrayElement(ArrayElement* p)
    {
       default_rule(p);
       check_array_element(p);
       //std::cout << "Visited aRRAY eLEmen"<< std::endl; 
    }

    // Special cases
    void visitPrimitive(Primitive* p) {}
    void visitSymName(SymName* p) {}
    void visitStringPrimitive(StringPrimitive* p) {
        //std::cout << "Visited String Primitive \n";
    }
};


void dopass_typecheck(Program_ptr ast, SymTab* st)
{
    Typecheck* typecheck = new Typecheck(stderr, st);
    ast->accept(typecheck); // Walk the tree with the visitor above
    delete typecheck;
}
