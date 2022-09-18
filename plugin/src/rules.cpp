#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include <algorithm>
#include <optional>
#include <set>
#include <sstream>

using namespace std;
using namespace clang;

// #define DEBUG_PLUGIN
#ifndef DEBUG_PLUGIN
#define DBG(...)
#define DBG_NOTE(...)
#else
#include <iostream>
#define DBG(expr)                                                      \
    do                                                                 \
    {                                                                  \
        cerr << "\033[33m" #expr ": "                                  \
             << boolalpha << (expr) << boolalpha << "\033[0m" << endl; \
    } while (false)
// ]]

#define DBG_NOTE(expr)                              \
    do                                              \
    {                                               \
        cerr << "\033[32m" #expr "\033[0m" << endl; \
    } while (false)
// ]]
#endif // DEBUG_PLUGIN

namespace
{
/**
 * @brief Get the first parent node of type `Parent` for `node` in the AST that matches
 *        the given predicate.
 *
 * @tparam Parent   The type of the parent node.
 * @param context   The AST context.
 * @param node      The starting node.
 * @param pred      A boolean predicate than accepts a `const Parent&`.
 *
 * @return const Parent*    Returns the first matching parent or `nullptr` if no such parent exists.
 */
template <typename Parent, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Parent &>>>
const Parent *GetParent(ASTContext &context, const ast_type_traits::DynTypedNode &node, const Predicate &pred)
{
    auto &&parents = context.getParents(node);
    for (auto &&dynamic_parent : parents)
    {
        if (const Parent *parent = dynamic_parent.get<Parent>();
            parent != nullptr && pred(*parent))
        {
            return parent;
        }
        if (const Parent *matching_ancestor = GetParent<Parent>(context, dynamic_parent, pred);
            matching_ancestor != nullptr)
        {
            return matching_ancestor;
        }
    }
    return nullptr;
}

template <typename Parent>
const Parent *GetParent(ASTContext &context, const ast_type_traits::DynTypedNode &node)
{
    return GetParent<Parent>(context, node, [](const Parent &) { return true; });
}

template <typename Parent, typename Node, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Parent &>>>
const Parent *GetParent(ASTContext &context, const Node &node, const Predicate &pred)
{
    return GetParent<Parent>(context, ast_type_traits::DynTypedNode::create(node), pred);
}

template <typename Parent, typename Node>
const Parent *GetParent(ASTContext &context, const Node &node)
{
    return GetParent<Parent>(context, ast_type_traits::DynTypedNode::create(node));
}

template <typename Parent, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Parent &>>>
bool HasParent(ASTContext &context, const ast_type_traits::DynTypedNode &node, const Predicate &pred)
{
    return GetParent<Parent>(context, node, pred) != nullptr;
}

template <typename Parent>
bool HasParent(ASTContext &context, const ast_type_traits::DynTypedNode &node)
{
    return GetParent<Parent>(context, node) != nullptr;
}

template <typename Parent, typename Node, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Parent &>>>
bool HasParent(ASTContext &context, const Node &node, const Predicate &pred)
{
    return GetParent<Parent>(context, node, pred) != nullptr;
}

template <typename Parent, typename Node>
bool HasParent(ASTContext &context, const Node &node)
{
    return GetParent<Parent>(context, node) != nullptr;
}

/**
 * @brief Get the first child node of type `Child` for `node` in the AST that matches
 *        the given predicate.
 *
 * @tparam Child    The type of the child node.
 * @param node      The starting node.
 * @param pred      A boolean predicate than accepts a `const Child&`.
 *
 * @return const Child* Returns the first matching child or `nullptr` if no such child exists.
 */
template <typename Child, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Child &>>>
const Child *GetChild(const Stmt &stmt, const Predicate &pred)
{
    for (auto &&child_stmt : stmt.children())
    {
        if (!child_stmt)
        {
            continue;
        }
        if (const auto *child = dyn_cast<Child>(child_stmt);
            child != nullptr && pred(*child))
        {
            return child;
        }
        if (const Child *matching_descendant = GetChild<Child>(*child_stmt, pred);
            matching_descendant != nullptr)
        {
            return matching_descendant;
        }
    }
    return nullptr;
}

template <typename Child>
const Child *GetChild(const Stmt &stmt)
{
    return GetChild<Child>(stmt, [](const Child &) { return true; });
}

template <typename Child, typename Predicate, typename = enable_if_t<is_invocable_r_v<bool, Predicate, const Child &>>>
bool HasChild(const Stmt &stmt, const Predicate &pred)
{
    return GetChild<Child>(stmt, pred) != nullptr;
}

template <typename Child>
bool HasChild(const Stmt &stmt)
{
    return GetChild<Child>(stmt) != nullptr;
}

struct FullLocationInfo
{
    const FullSourceLoc start;
    const optional<SourceRange> code{nullopt};
};

struct LocationInfo
{
    const SourceLocation start;
    const optional<SourceRange> code{nullopt};

    LocationInfo(const SourceLocation loc) : start{loc} {}
    LocationInfo(const SourceRange range) : start{range.getBegin()}, code{range} {}
    LocationInfo(const SourceLocation loc, const SourceRange range) : start{loc}, code{range} {}
};

template <size_t N>
void EmitDiagnosticMessage(ASTContext &context, DiagnosticsEngine::Level level, const FullLocationInfo loc, const char (&message)[N])
{
    auto &&diag_engine{context.getDiagnostics()};
    const auto id{diag_engine.getCustomDiagID(level, message)};
    if (loc.code)
    {
        const auto diag{diag_engine.Report(loc.start, id)};
        diag.AddSourceRange(CharSourceRange{*loc.code, false});
    }
    else
    {
        const auto diag{diag_engine.Report(id)};
    }
}

template <size_t N>
void Report(ASTContext &context, const LocationInfo where, const LocationInfo what, const char (&message)[N])
{
    const auto full_where = FullLocationInfo{context.getFullLoc(where.start), where.code};
    EmitDiagnosticMessage(context, DiagnosticsEngine::Error, {context.getFullLoc(what.start), what.code}, message);
    EmitDiagnosticMessage(context, DiagnosticsEngine::Note, full_where, "in function:");
    EmitDiagnosticMessage(context, DiagnosticsEngine::Note, {full_where.start}, "a standard algorithm from the <algorithm> or <numeric> headers might help.");
}

class NoRecursionVisitor : public RecursiveASTVisitor<NoRecursionVisitor>
{
public:
    bool VisitFunctionDecl(FunctionDecl *decl)
    {
        if (!decl->hasBody() || !decl->isDefined() || decl != decl->getDefinition())
        {
            return true;
        }

        if (const auto full_location{decl->getASTContext().getFullLoc(decl->getLocation())};
            full_location.isInSystemHeader())
        {
            return true;
        }

        struct
        {
            const FunctionDecl *decl;
            mutable set<const FunctionDecl *> seen{};
            auto operator()(const CallExpr &expr) const
            {
                const auto *const func_decl = expr.getDirectCallee();
                if (!func_decl)
                {
                    return false;
                }

                if (const auto *const first_decl = func_decl->getFirstDecl();
                    seen.find(func_decl) == seen.end())
                {
                    seen.insert(func_decl);
                    return first_decl == decl ||
                           (func_decl->hasBody() && HasChild<CallExpr>(*func_decl->getBody(), *this));
                }
                return false;
            }
        } is_self_call_expr{decl->getFirstDecl()};
        if (const auto *recursive_call = GetChild<CallExpr>(*decl->getBody(), is_self_call_expr))
        {
            Report(decl->getASTContext(), {decl->getSourceRange()}, {recursive_call->getExprLoc(), recursive_call->getSourceRange()},
                   "recursion is not allowed in this stage.");
        }

        return true;
    }
};

class NoLoopsVisitor : public RecursiveASTVisitor<NoLoopsVisitor>
{
public:
    static constexpr size_t max_message = 100;
    void HandleLoopStmt(const Stmt *loop, const char *loop_kind = nullptr)
    {
        const auto full_location{ctx->getFullLoc(loop->getBeginLoc())};
        if (full_location.isInSystemHeader())
        {
            return;
        }

        char msg[max_message]{};

        {
            stringstream msg_builder;
            if (loop_kind)
            {
                msg_builder << loop_kind << ' ';
            }
            msg_builder << "loops are not allowed in this stage.";
            msg_builder.get(msg, sizeof(msg));
        }

        Report(*ctx, GetParent<FunctionDecl>(*ctx, *loop)->getSourceRange(), loop->getSourceRange(), msg);
    }

    bool VisitWhileStmt(WhileStmt *while_stmt)
    {
        HandleLoopStmt(while_stmt, "while");
        return true;
    }

    bool VisitDoStmt(DoStmt *do_stmt)
    {
        HandleLoopStmt(do_stmt, "do-while");
        return true;
    }

    bool VisitForStmt(ForStmt *for_stmt)
    {
        HandleLoopStmt(for_stmt, "for");
        return true;
    }

    bool VisitCXXForRangeStmt(CXXForRangeStmt *for_stmt)
    {
        HandleLoopStmt(for_stmt, "range-based for");
        return true;
    }

    bool VisitGotoStmt(GotoStmt *goto_stmt)
    {
        if (ctx->getFullLoc(goto_stmt->getLabel()->getLocation()) > ctx->getFullLoc(goto_stmt->getGotoLoc()))
        {
            return true;
        }

        HandleLoopStmt(goto_stmt, "goto-based");
        return true;
    }

    bool VisitCallExpr(CallExpr *call_expr)
    {
        const auto full_location{ctx->getFullLoc(call_expr->getBeginLoc())};
        if (full_location.isInSystemHeader())
        {
            return true;
        }

        const auto callee_name = [=] {
            if (const auto *const callee = call_expr->getDirectCallee(); callee && callee->getName() == "for_each")
            {
                return callee->getNameAsString();
            }
            if (const auto *const callee = dyn_cast<UnresolvedLookupExpr>(*call_expr->child_begin()); callee)
            {
                return callee->getName().getAsString();
            }
            return ""s;
        }();

        if (callee_name != "for_each")
        {
            return true;
        }

        const auto func_location = [=]() -> SourceRange {
            const auto *const func_decl = GetParent<FunctionDecl>(*ctx, *call_expr);
            if (!func_decl)
            {
                return {};
            }
            if (func_decl->isOverloadedOperator() && func_decl->getOverloadedOperator() == OverloadedOperatorKind::OO_Call)
            {
                if (const auto *const lambda = GetParent<LambdaExpr>(*ctx, *func_decl); lambda)
                {
                    if (const auto *const lambda_obj = GetParent<VarDecl>(*ctx, *lambda))
                    {
                        return {lambda_obj->getLocation(), lambda_obj->getLocation().getLocWithOffset(lambda_obj->getName().size())};
                    }
                    return lambda->getSourceRange();
                }
            }
            return func_decl->getSourceRange();
        }();

        Report(*ctx, func_location, call_expr->getSourceRange(),
               "the for_each algorithm is not allowed in this stage.");
        return true;
    }

    void SetASTContext(ASTContext &context)
    {
        ctx = &context;
    }

    ASTContext *ctx{nullptr};
};

class NoAsmVisitor : public RecursiveASTVisitor<NoAsmVisitor>
{
public:
    bool VisitAsmStmt(AsmStmt *stmt)
    {
        const auto full_location{ctx->getFullLoc(stmt->getAsmLoc())};
        if (full_location.isInSystemHeader())
        {
            return true;
        }

        Report(*ctx, GetParent<FunctionDecl>(*ctx, *stmt)->getSourceRange(), stmt->getSourceRange(),
               "inline assembly is not allowed in this stage.");

        return true;
    }

    void SetASTContext(ASTContext &context)
    {
        ctx = &context;
    }

    ASTContext *ctx{nullptr};
};

class WarningVisitor : public RecursiveASTVisitor<WarningVisitor>
{
public:
    template <size_t N>
    static void Warn(ASTContext &context, const LocationInfo where, const LocationInfo what, const char (&message)[N])
    {
        const auto full_where = FullLocationInfo{context.getFullLoc(where.start), where.code};
        EmitDiagnosticMessage(context, DiagnosticsEngine::Warning, {context.getFullLoc(what.start), what.code}, message);
        EmitDiagnosticMessage(context, DiagnosticsEngine::Note, full_where, "in function:");
    }

    bool VisitVarDecl(VarDecl *decl)
    {
        if (const auto full_location{decl->getASTContext().getFullLoc(decl->getLocation())};
            full_location.isInSystemHeader())
        {
            return true;
        }

        DBG(decl->getNameAsString());

        if (!decl->isLocalVarDeclOrParm() || decl->isLocalVarDecl())
        {
            DBG_NOTE(not param);
            return true;
        }

        const auto type = decl->getType();
        if (type->isReferenceType()) // || !type.isConstQualified())
        {
            DBG_NOTE(const ref);
            return true;
        }

        DBG(type.getCanonicalType().getUnqualifiedType().getAsString());
        if (type.getCanonicalType().getUnqualifiedType().getAsString() != "std::basic_string")
        {
            DBG_NOTE(wtf);
            return true;
        }

        Warn(*ctx, GetParent<FunctionDecl>(*ctx, *decl)->getSourceRange(), decl->getSourceRange(),
             "prefer passing strings by const reference rather than by value.");

        return true;
    }

    void SetASTContext(ASTContext &context)
    {
        ctx = &context;
    }

    ASTContext *ctx{nullptr};
};

template <typename T, typename = void>
struct needs_ast : false_type
{
};

template <typename T>
struct needs_ast<T, void_t<decltype(declval<T>().SetASTContext(declval<ASTContext &>()))>> : true_type
{
};

template <typename Visitor>
class ClassicConsumer : public ASTConsumer
{
public:
    virtual void HandleTranslationUnit(ASTContext &context) override
    {
        if constexpr (needs_ast<Visitor>::value)
        {
            visitor.SetASTContext(context);
        }
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

private:
    Visitor visitor;
};

template <typename Consumer>
class ClassicASTAction : public PluginASTAction
{
public:
    virtual unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, llvm::StringRef InFile) override
    {
        return make_unique<Consumer>();
    }

    virtual bool ParseArgs(const CompilerInstance &CI,
                           const vector<string> &args) override
    {
        return true;
    }

    virtual ActionType getActionType() override { return AddBeforeMainAction; }
};

} // namespace

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)
#define REGISTER_VISITOR(type, name, desc) \
    static clang::FrontendPluginRegistry::Add<ClassicASTAction<ClassicConsumer<type>>> CAT(plugin_, __LINE__)(name, desc)

REGISTER_VISITOR(NoRecursionVisitor, "no-recursion", "Disallow recursion.");
REGISTER_VISITOR(NoLoopsVisitor, "no-loops", "Disallow loops.");
REGISTER_VISITOR(NoAsmVisitor, "no-asm", "Disallow inline assembly.");
REGISTER_VISITOR(WarningVisitor, "warnings", "Additional warnings.");
