namespace Compiler.Lexer;

// $TODO add FFT types
internal enum TokenType
{
  // An invalid token
  Invalid,

  // This is used for convenience so we don't have to explicitly check the token array length
  [TokenDescription("end of file")] EndOfFile,

  // Keywords
  [KeywordToken("import")] KeywordImport,
  [KeywordToken("export")] KeywordExport,
  [KeywordToken("as")] KeywordAs,
  [KeywordToken("struct")] KeywordStruct,
  [KeywordToken("val")] KeywordVal,
  [KeywordToken("module")] KeywordModule,
  [KeywordToken("const")] KeywordConst,
  [KeywordToken("in")] KeywordIn,
  [KeywordToken("out")] KeywordOut,
  [KeywordToken("return")] KeywordReturn,
  [KeywordToken("if")] KeywordIf,
  [KeywordToken("else")] KeywordElse,
  [KeywordToken("for")] KeywordFor,
  [KeywordToken("break")] KeywordBreak,
  [KeywordToken("continue")] KeywordContinue,

  // Primitive type keywords (and void)
  [KeywordToken("void")] KeywordVoid,
  [KeywordToken("float")] KeywordFloat,
  [KeywordToken("double")] KeywordDouble,
  [KeywordToken("int")] KeywordInt,
  [KeywordToken("bool")] KeywordBool,
  [KeywordToken("string")] KeywordString,

  // Identifiers start with [a-zA-Z_] followed by 0 or more [a-zA-Z0-9_]
  [TokenDescription("identifier")] Identifier,

  // Literals
  [TokenDescription("float literal")] LiteralFloat,
  [TokenDescription("double literal")] LiteralDouble,
  [TokenDescription("int literal")] LiteralInt,
  [TokenDescription("bool literal")] LiteralBool,
  [TokenDescription("string literal")] LiteralString,

  // Integer upsample factor (e.g. 2x)
  [TokenDescription("upsample factor")] UpsampleFactor,

  // Braces
  [SymbolToken("{")] LeftBrace,
  [SymbolToken("}")] RightBrace,

  [SymbolToken("@")] At,    // Used to force a native import and to indicate upsampled data types
  [SymbolToken("#")] Pound, // Prefix to instrument properties
  [SymbolToken(".")] Period,
  [SymbolToken(",")] Comma,
  [SymbolToken(":")] Colon,
  [SymbolToken(";")] Semicolon,
  [SymbolToken("?")] QuestionMark,

  // Assignment operators
  [SymbolToken("=")] Assign,
  [SymbolToken("+=")] [AssignmentOperatorToken(Plus)] AssignPlus,
  [SymbolToken("-=")] [AssignmentOperatorToken(Minus)] AssignMinus,
  [SymbolToken("*=")] [AssignmentOperatorToken(Multiply)] AssignMultiply,
  [SymbolToken("/=")] [AssignmentOperatorToken(Divide)] AssignDivide,
  [SymbolToken("%=")] [AssignmentOperatorToken(Modulo)] AssignModulo,
  [SymbolToken("&=")] [AssignmentOperatorToken(BitwiseAnd)] AssignAnd,
  [SymbolToken("|=")] [AssignmentOperatorToken(BitwiseOr)] AssignOr,

  // Math/logic operators
  [SymbolToken("||")] LogicalOr,
  [SymbolToken("&&")] LogicalAnd,
  [SymbolToken("|")] BitwiseOr,
  [SymbolToken("^")] BitwiseXor,
  [SymbolToken("&")] BitwiseAnd,
  [SymbolToken("==")] Equal,
  [SymbolToken("!=")] NotEqual,
  [SymbolToken("<")] LessThan,
  [SymbolToken(">")] GreaterThan,
  [SymbolToken("<=")] LessThanEqual,
  [SymbolToken(">=")] GreaterThanEqual,
  [SymbolToken("+")] Plus,
  [SymbolToken("-")] Minus,
  [SymbolToken("*")] Multiply,
  [SymbolToken("/")] Divide,
  [SymbolToken("%")] Modulo,
  [SymbolToken("!")] LogicalNot,
  [SymbolToken("~")] BitwiseNot,

  // Parentheses
  [SymbolToken("(")] LeftParenthesis,
  [SymbolToken(")")] RightParenthesis,

  // Brackets
  [SymbolToken("[")] LeftBracket,
  [SymbolToken("]")] RightBracket,
}

[AttributeUsage(AttributeTargets.Field)]
internal class KeywordTokenAttribute(string keyword) : Attribute
{
  public string Keyword { get; } = keyword;
}

[AttributeUsage(AttributeTargets.Field)]
internal class SymbolTokenAttribute(string text) : Attribute
{
  public string Text { get; } = text;
}

[AttributeUsage(AttributeTargets.Field)]
internal class AssignmentOperatorTokenAttribute(TokenType operatorTokenType) : Attribute
{
  public TokenType OperatorTokenType { get; } = operatorTokenType;
}

[AttributeUsage(AttributeTargets.Field)]
internal class TokenDescriptionAttribute(string description) : Attribute
{
  public string Description { get; } = description;
}