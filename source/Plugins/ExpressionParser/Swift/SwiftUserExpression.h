//===-- SwiftUserExpression.h -----------------------------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_SwiftUserExpression_h_
#define liblldb_SwiftUserExpression_h_

// C Includes
// C++ Includes
#include <string>
#include <map>
#include <vector>

#include "lldb/Expression/Materializer.h"
#include "lldb/Expression/LLVMUserExpression.h"

// Other libraries and framework includes
// Project includes

namespace lldb_private
{
    
//----------------------------------------------------------------------
/// @class SwiftUserExpression SwiftUserExpression.h "lldb/Expression/SwiftUserExpression.h"
/// @brief Encapsulates a single expression for use with Clang
///
/// LLDB uses expressions for various purposes, notably to call functions
/// and as a backend for the expr command.  SwiftUserExpression encapsulates
/// the objects needed to parse and interpret or JIT an expression.  It
/// uses the Clang parser to produce LLVM IR from the expression.
//----------------------------------------------------------------------
class SwiftUserExpression : public LLVMUserExpression
{
public:
    enum { kDefaultTimeout = 500000u };
    
    class SwiftUserExpressionHelper : public ExpressionTypeSystemHelper
    {
    public:
        SwiftUserExpressionHelper (Target &target) :
            ExpressionTypeSystemHelper(eKindSwiftHelper),
            m_target(target)
        {
        }
        
        ~SwiftUserExpressionHelper() {}
    private:
        Target &m_target;
    };
    
    //------------------------------------------------------------------
    /// Constructor
    ///
    /// @param[in] expr
    ///     The expression to parse.
    ///
    /// @param[in] expr_prefix
    ///     If non-NULL, a C string containing translation-unit level
    ///     definitions to be included when the expression is parsed.
    ///
    /// @param[in] language
    ///     If not eLanguageTypeUnknown, a language to use when parsing
    ///     the expression.  Currently restricted to those languages
    ///     supported by Clang.
    ///
    /// @param[in] desired_type
    ///     If not eResultTypeAny, the type to use for the expression
    ///     result.
    ///
    /// @param[in] options
    ///     Additional options for the expression.
    //------------------------------------------------------------------
    SwiftUserExpression (ExecutionContextScope &exe_scope,
                         const char *expr,
                         const char *expr_prefix,
                         lldb::LanguageType language,
                         ResultType desired_type,
                         const EvaluateExpressionOptions &options);
    
    //------------------------------------------------------------------
    /// Destructor
    //------------------------------------------------------------------
    ~SwiftUserExpression() override;
    
    //------------------------------------------------------------------
    /// Parse the expression
    ///
    /// @param[in] error_stream
    ///     A stream to print parse errors and warnings to.
    ///
    /// @param[in] exe_ctx
    ///     The execution context to use when looking up entities that
    ///     are needed for parsing (locations of functions, types of
    ///     variables, persistent variables, etc.)
    ///
    /// @param[in] execution_policy
    ///     Determines whether interpretation is possible or mandatory.
    ///
    /// @param[in] keep_result_in_memory
    ///     True if the resulting persistent variable should reside in
    ///     target memory, if applicable.
    ///
    /// @return
    ///     True on success (no errors); false otherwise.
    //------------------------------------------------------------------
    bool
    Parse (Stream &error_stream,
           ExecutionContext &exe_ctx,
           lldb_private::ExecutionPolicy execution_policy,
           bool keep_result_in_memory,
           bool generate_debug_info,
           uint32_t line_offset = 0) override;
    
    ExpressionTypeSystemHelper *
    GetTypeSystemHelper () override
    {
        return &m_type_system_helper;
    }
    
    Materializer::PersistentVariableDelegate &
    GetResultDelegate()
    {
        return m_result_delegate;
    }
    
    Materializer::PersistentVariableDelegate &
    GetErrorDelegate()
    {
        return m_error_delegate;
    }
    
    Materializer::PersistentVariableDelegate &
    GetPersistentVariableDelegate()
    {
        return m_persistent_variable_delegate;
    }
    
    lldb::ExpressionVariableSP
    GetResultAfterDematerialization(ExecutionContextScope *exe_scope) override;
    
private:
    //------------------------------------------------------------------
    /// Populate m_in_cplusplus_method and m_in_objectivec_method based on the environment.
    //------------------------------------------------------------------
    
    void
    ScanContext (ExecutionContext &exe_ctx,
                 lldb_private::Error &err) override;
    
    bool
    AddArguments (ExecutionContext &exe_ctx,
                  std::vector<lldb::addr_t> &args,
                  lldb::addr_t struct_address,
                  Stream &error_stream) override;
        
    SwiftUserExpressionHelper   m_type_system_helper;
    
    class ResultDelegate : public Materializer::PersistentVariableDelegate
    {
    public:
        ResultDelegate(SwiftUserExpression &, bool is_error);
        ConstString GetName() override;
        void DidDematerialize(lldb::ExpressionVariableSP &variable) override;
        
        void RegisterPersistentState(PersistentExpressionState *persistent_state);
        lldb::ExpressionVariableSP &GetVariable();
    private:
        SwiftUserExpression &m_user_expression;
        bool m_is_error;
        PersistentExpressionState *m_persistent_state;
        lldb::ExpressionVariableSP m_variable;
    };
    
    ResultDelegate m_result_delegate;
    ResultDelegate m_error_delegate;
    
    class PersistentVariableDelegate : public Materializer::PersistentVariableDelegate
    {
    public:
        PersistentVariableDelegate(SwiftUserExpression &);
        ConstString GetName() override;
        void DidDematerialize(lldb::ExpressionVariableSP &variable) override;
    private:
        SwiftUserExpression &m_user_expression;
    };
    
    PersistentVariableDelegate m_persistent_variable_delegate;
};
    
} // namespace lldb_private

#endif // liblldb_SwiftUserExpression_h_
