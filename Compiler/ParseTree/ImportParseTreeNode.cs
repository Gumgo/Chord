using Compiler.Parsing;
using Compiler.Tokenization;
using System.Diagnostics;

namespace Compiler.ParseTree;

internal class ImportParseTreeNode : ParseTreeNode
{
  private const string _component = "import";

  private ImportParseTreeNode(
    SourceLocation sourceLocation,
    bool isNative,
    int? parentDirectoryCount,
    IReadOnlyList<string> pathComponents,
    IReadOnlyList<string>? importAsComponents)
    : base(sourceLocation)
  {
    IsNative = isNative;
    ParentDirectoryCount = parentDirectoryCount;
    PathComponents = pathComponents;
    ImportAsComponents = importAsComponents;
  }

  public bool IsNative { get; }
  public int? ParentDirectoryCount { get; } // If null, this imports from the root source directory
  public IReadOnlyList<string> PathComponents { get; }
  public IReadOnlyList<string>? ImportAsComponents { get; }

  public static bool CanParse(Token nextToken)
    => nextToken.TokenType == TokenType.KeywordImport;

  public static ImportParseTreeNode Parse(ParserContext context, ParserLocation location)
  {
    Debug.Assert(CanParse(location.NextToken()));
    var tokenSourceLocations = new List<SourceLocation>();
    location.ConsumeNextToken(tokenSourceLocations);

    int? parentDirectoryCount = null;
    var pathComponents = new List<string>();

    // Parse the import path. Import paths use the following rules:
    //   a.b.c - search from top-level source directory ($TODO add installed package location too)
    //   .a.b.c - search from current directory
    //   ..a.b.c - search from parent directory (each additional . moves up the directory tree one level)
    //   @library - native library import (must be a single identifier)
    var isNative = location.ConsumeIfNextTokenIs(TokenType.At, tokenSourceLocations) != null;
    if (isNative)
    {
      var pathComponent = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
      if (pathComponent == null)
      {
        context.Reporting.MalformedError(_component, location.NextToken(), "native import path");
        location.RecoverFromError();
      }

      pathComponents.Add(pathComponent);
    }
    else
    {
      if (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
      {
        parentDirectoryCount = 0;
        while (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
        {
          parentDirectoryCount++;
        }
      }

      {
        var pathComponent = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
        if (pathComponent == null)
        {
          context.Reporting.MalformedError(_component, location.NextToken(), "import path");
          location.RecoverFromError();
        }

        pathComponents.Add(pathComponent);
      }

      while (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
      {
        var pathComponent = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
        if (pathComponent == null)
        {
          context.Reporting.MalformedError(_component, location.NextToken(), "import path");
          location.RecoverFromError();
        }

        pathComponents.Add(pathComponent);
      }
    }

    // Optionally, the following can be added after the import path:
    //   as . - imports everything to the root scope ('foo.bar' becomes just 'bar')
    //   as a.b.c - imports into a custom scope ('foo.bar' becomes 'a.b.c.bar')
    List<string>? importAsComponents = null;
    if (location.ConsumeIfNextTokenIs(TokenType.KeywordAs, tokenSourceLocations) != null)
    {
      if (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
      {
        importAsComponents = [];
      }
      else
      {
        {
          var importAsComponent = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
          if (importAsComponent == null)
          {
            context.Reporting.MalformedError(_component, location.NextToken(), "scope alias");
            location.RecoverFromError();
          }

          importAsComponents = [importAsComponent];
        }

        while (location.ConsumeIfNextTokenIs(TokenType.Period, tokenSourceLocations) != null)
        {
          var importAsComponent = location.ConsumeIfNextTokenIs(TokenType.Identifier, tokenSourceLocations)?.IdentifierValue;
          if (importAsComponent == null)
          {
            context.Reporting.MalformedError(_component, location.NextToken(), "scope alias");
            location.RecoverFromError();
          }

          importAsComponents.Add(importAsComponent);
        }
      }
    }

    if (location.ConsumeIfNextTokenIs(TokenType.Semicolon, tokenSourceLocations) == null)
    {
      context.Reporting.MalformedError(_component, location.NextToken(), TokenType.Semicolon);
      location.RecoverFromError();
    }

    return new(tokenSourceLocations.Merge(), isNative, parentDirectoryCount, pathComponents, importAsComponents);
  }
}