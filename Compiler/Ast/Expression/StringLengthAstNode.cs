﻿using Compiler.Types;

namespace Compiler.Ast.Expression;

internal class StringLengthAstNode(SourceLocation sourceLocation, ExpressionAstNode expression)
  : ExpressionAstNode(sourceLocation)
{
  public override string NodeName => "string length";
  public override AstDataType DataType { get; } = new(RuntimeMutability.Constant, PrimitiveType.Int, 1, false);
  public override bool IsWritableReference => false;
  public override bool IsGlobalReference => false;

  public ExpressionAstNode Expression => expression;
}