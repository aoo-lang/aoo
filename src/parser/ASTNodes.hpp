#pragma once
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <vector>

namespace AOO::Parser {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    using std::span, std::vector;

    enum struct NodeKind : u16 {
        //Top-level / program structure
        Module,             //payload: 0; children: ModuleDecl?, Imports..., Decls...
        ModuleDecl,         //tokenIndex: name; children: none
        ImportDecl,         //children: ImportItem...
        ImportPath,         //children: PathSegments...
        ImportSegment,      //tokenIndex: ident
        ImportItem,         //tokenIndex: imported leaf ident; children: full ImportPath
        TypeDecl,           //tokenIndex: name; payload: genericParamCount; children: GenericParams..., bodyItems...
        TraitDecl,          //tokenIndex: name; payload: genericParamCount; children: GenericParams..., constraintExpr?, bodyItems...
        EnumDecl,           //tokenIndex: name; children: EnumVariants...
        EnumVariant,        //tokenIndex: name; children: payloadType?
        FunctionDecl,       //tokenIndex: name; payload: see below; children: returnType, GenericParams..., Params..., body
        OperatorDecl,       //tokenIndex: op token; payload: operator kind; children: returnType?, Params..., body
        FieldDecl,          //tokenIndex: name; children: type, init?
        VariableDecl,       //tokenIndex: name; children: type, init?
        DupDecl,            //tokenIndex: KW_DUP; children: init?
        VisibilityLabel,    //tokenIndex: keyword (KW_PUBLIC/KW_PRIVATE)
        GenericParam,       //tokenIndex: ident; payload: kind (0=type, 1=value, 2=trait); children: optional bound/default
        ParamList,          //children: Param...
        Param,              //tokenIndex: name (or 0 for self); children: type
        SelfParam,          //tokenIndex: KW_SELF; payload: flags (1=mut "!", 2=consumed "~")

        //Statements
        Block,              //children: stmts...
        ExprStmt,           //children: expr
        IfStmt,             //children: cond, thenBlock, elseBlock?
        ForStmt,            //children: init, cond?, step?, body  (C-style) OR pattern, range, body (range-style)
        MatchStmt,          //children: discriminant, MatchArm...
        MatchArm,           //children: pattern, body
        ReturnStmt,         //children: expr?
        BreakStmt,          //tokenIndex: optional label
        ContinueStmt,       //tokenIndex: optional label

        //Expressions: leaves
        IntLit,             //payload: numeric value; tokenIndex: literal token
        FloatLit,           //payload: bit-cast double
        StringLit,          //tokenIndex: literal token
        CharLit,            //payload: char value
        LabelLit,           //tokenIndex: label token
        BoolLit,            //payload: 0/1 — currently no bool keyword; reserved
        Identifier,         //tokenIndex: ident token
        SelfExpr,           //tokenIndex: KW_SELF
        VoidExpr,           //tokenIndex: KW_VOID

        //Expressions: structural
        BinaryOp,           //tokenIndex: op token; payload: TokenType; children: left, right
        UnaryPrefix,        //tokenIndex: op token; payload: TokenType; children: operand
        UnaryPostfix,       //tokenIndex: op token; payload: TokenType; children: operand
        TernaryQ,           //tokenIndex: '?:' token; children: cond, thenE, elseE
        TernaryQQ,          //tokenIndex: '??' token; children: cond, fallback
        Call,               //children: callee, args...
        Index,              //children: target, index
        MemberAccess,       //tokenIndex: name token; children: target
        ArrowAccess,        //tokenIndex: name token; children: target
        ScopeAccess,        //tokenIndex: name token; children: target
        DoubleScopeAccess,  //tokenIndex: name token; children: target
        GenericApp,         //children: base, typeArgs...  -- typeArgs are type-mode nodes
        StructLit,          //children: typeExpr, StructLitField...
        StructLitField,     //tokenIndex: name; payload: 0=pun (k), 1=assign (k=v), 2=copy-init (k:=v); children: value?
        AsCast,             //children: expr, type
        Range,              //tokenIndex: '..' or '...'; payload: TokenType; children: low, high
        Dup,                //children: operand

        //Type expressions
        TypeIdent,          //tokenIndex: ident or keyword
        TypePtr,            //children: inner   (T*)
        TypeRef,            //children: inner   (T&)
        TypeMut,            //children: inner   (T!)
        TypeMustInit,       //children: inner   (T!!)
        TypeConsumed,       //children: inner   (~T)
        TypeAuto,           //tokenIndex: KW_AUTO
        TypeVoid,           //tokenIndex: KW_VOID

        //Diagnostics
        Error,

        Count
    };

    //Node flag bits.
    constexpr u16 FLAG_PUBLIC    = 1u << 0;
    constexpr u16 FLAG_PRIVATE   = 1u << 1;
    constexpr u16 FLAG_EXPORT    = 1u << 2;
    constexpr u16 FLAG_HAS_ERROR = 1u << 3;
    constexpr u16 FLAG_SYNTHETIC = 1u << 4;
    //Function-decl shape flags (placed in payload, bit 0..7):
    constexpr u64 FN_PAYLOAD_EXPR_BODY = 1ull << 0;  //body is a single expression (=> ...)
    constexpr u64 FN_PAYLOAD_HAS_SELF  = 1ull << 1;

    struct ASTNode {
        NodeKind kind;        //2 B
        u16      flags;       //2 B
        u32      tokenIndex;  //4 B — into the lexer token vector
        u32      firstChild;  //4 B — into AST::childIndices
        u32      childCount;  //4 B
        u64      payload;     //8 B — kind-specific
    };
    static_assert(sizeof(ASTNode) == 24, "ASTNode must be 24 bytes");

    struct AST {
        vector<ASTNode> nodes;
        vector<u32>     childIndices;
        u32             root{0};
    };

    [[nodiscard]] inline u32 addNode(AST& ast, ASTNode node) noexcept {
        const u32 idx = static_cast<u32>(ast.nodes.size());
        ast.nodes.push_back(node);
        return idx;
    }

    //Reserve a span in childIndices for `count` children, returning the start.
    //The caller writes into ast.childIndices[start .. start+count) directly.
    [[nodiscard]] inline u32 reserveChildren(AST& ast, u32 count) noexcept {
        const u32 start = static_cast<u32>(ast.childIndices.size());
        ast.childIndices.resize(start + count);
        return start;
    }

    //Append children from a temporary list (ordered).
    [[nodiscard]] inline u32 appendChildren(AST& ast, const vector<u32>& children) noexcept {
        const u32 start = static_cast<u32>(ast.childIndices.size());
        ast.childIndices.insert(ast.childIndices.end(), children.begin(), children.end());
        return start;
    }

    [[nodiscard]] inline span<const u32> childrenOf(const AST& ast, const ASTNode& node) noexcept {
        if (node.childCount == 0) return {};
        return span<const u32>(ast.childIndices.data() + node.firstChild, node.childCount);
    }

    template<typename T>
    [[nodiscard]] inline T payloadAs(const ASTNode& node) noexcept {
        static_assert(std::is_trivially_copyable_v<T>);
        static_assert(sizeof(T) <= sizeof(node.payload));
        if constexpr (sizeof(T) == sizeof(node.payload)) return std::bit_cast<T>(node.payload);
        else {
            T result{};
            std::memcpy(&result, &node.payload, sizeof(T));
            return result;
        }
    }

    [[nodiscard]] inline const char* tostring_NodeKind(NodeKind kind) noexcept {
        using enum NodeKind;
        switch (kind) {
            case Module: return "Module";
            case ModuleDecl: return "ModuleDecl";
            case ImportDecl: return "ImportDecl";
            case ImportPath: return "ImportPath";
            case ImportSegment: return "ImportSegment";
            case ImportItem: return "ImportItem";
            case TypeDecl: return "TypeDecl";
            case TraitDecl: return "TraitDecl";
            case EnumDecl: return "EnumDecl";
            case EnumVariant: return "EnumVariant";
            case FunctionDecl: return "FunctionDecl";
            case OperatorDecl: return "OperatorDecl";
            case FieldDecl: return "FieldDecl";
            case VariableDecl: return "VariableDecl";
            case DupDecl: return "DupDecl";
            case VisibilityLabel: return "VisibilityLabel";
            case GenericParam: return "GenericParam";
            case ParamList: return "ParamList";
            case Param: return "Param";
            case SelfParam: return "SelfParam";
            case Block: return "Block";
            case ExprStmt: return "ExprStmt";
            case IfStmt: return "IfStmt";
            case ForStmt: return "ForStmt";
            case MatchStmt: return "MatchStmt";
            case MatchArm: return "MatchArm";
            case ReturnStmt: return "ReturnStmt";
            case BreakStmt: return "BreakStmt";
            case ContinueStmt: return "ContinueStmt";
            case IntLit: return "IntLit";
            case FloatLit: return "FloatLit";
            case StringLit: return "StringLit";
            case CharLit: return "CharLit";
            case LabelLit: return "LabelLit";
            case BoolLit: return "BoolLit";
            case Identifier: return "Identifier";
            case SelfExpr: return "SelfExpr";
            case VoidExpr: return "VoidExpr";
            case BinaryOp: return "BinaryOp";
            case UnaryPrefix: return "UnaryPrefix";
            case UnaryPostfix: return "UnaryPostfix";
            case TernaryQ: return "TernaryQ";
            case TernaryQQ: return "TernaryQQ";
            case Call: return "Call";
            case Index: return "Index";
            case MemberAccess: return "MemberAccess";
            case ArrowAccess: return "ArrowAccess";
            case ScopeAccess: return "ScopeAccess";
            case DoubleScopeAccess: return "DoubleScopeAccess";
            case GenericApp: return "GenericApp";
            case StructLit: return "StructLit";
            case StructLitField: return "StructLitField";
            case AsCast: return "AsCast";
            case Range: return "Range";
            case Dup: return "Dup";
            case TypeIdent: return "TypeIdent";
            case TypePtr: return "TypePtr";
            case TypeRef: return "TypeRef";
            case TypeMut: return "TypeMut";
            case TypeMustInit: return "TypeMustInit";
            case TypeConsumed: return "TypeConsumed";
            case TypeAuto: return "TypeAuto";
            case TypeVoid: return "TypeVoid";
            case Error: return "Error";
            case Count: return "Count";
        }
    }
}