using Compiler.Tokenization;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Compiler.Parsing;

internal enum ErrorRecoveryBehavior
{
  AllowOuterScopeRecovery,
  BlockOuterScopeRecovery,
}

internal static class ErrorRecovery
{
  public delegate bool CanRecover(Token? previousToken, Token nextToken);

  public static void Run(ErrorRecoveryBehavior behavior, CanRecover canRecover, Action parseFunc, Action? onRecoverFromErrorFunc = null)
    => Run<object?>(
      behavior,
      canRecover,
      () =>
      {
        parseFunc();
        return null;
      },
      () =>
      {
        onRecoverFromErrorFunc?.Invoke();
        return null;
      });

  public static TResult Run<TResult>(ErrorRecoveryBehavior behavior, CanRecover canRecover, Func<TResult> parseFunc, Func<TResult> onRecoverFromErrorFunc)
  {
    using var errorRecoveryScope = new ErrorRecoveryScope(behavior, canRecover);

    try
    {
      return parseFunc();
    }
    catch (ErrorRecoveryException ex) when (ex.ErrorRecoveryScope == errorRecoveryScope)
    {
      // We recovered so remove the scope immediately in case more parsing happens in onRecoverFromErrorFunc (though this is more for correctness, I can't think
      // of an actual scenario where more parsing would occur here).
      errorRecoveryScope.Dispose();
      return onRecoverFromErrorFunc();
    }
  }
}

internal static class ParserLocationErrorRecoveryExtensions
{
  // This skips forward to the next error recovery token (if one is found), then throws an ErrorRecoveryException
  [DoesNotReturn]
  public static void RecoverFromError(this ParserLocation location)
  {
    // Advance until we find an error recovery token
    Token? previousToken = null;
    ErrorRecoveryScope? errorRecoveryScope = null;
    while (errorRecoveryScope == null)
    {
      errorRecoveryScope = ErrorRecoveryScope.TryToRecover(previousToken, location.NextToken());
      if (errorRecoveryScope == null)
      {
        previousToken = location.NextToken();
        location.Advance();

        // We should never skip past the last token because at the root level we should have an error recovery scope using the EndOfFile token type.
        Debug.Assert(location.TokenIndex < location.Tokens.Count);
      }
    }

    // Throw an exception so we pick up at the point of error recovery
    throw new ErrorRecoveryException(errorRecoveryScope);
  }
}

file sealed class ErrorRecoveryScope : IDisposable
{
  [ThreadStatic]
  private static List<ErrorRecoveryScope>? _activeErrorRecoveryScopes;

  private readonly ErrorRecoveryBehavior _behavior;
  private readonly ErrorRecovery.CanRecover _canRecover;
  private bool _disposed;

  public ErrorRecoveryScope(ErrorRecoveryBehavior behavior, ErrorRecovery.CanRecover canRecover)
  {
    _behavior = behavior;
    _canRecover = canRecover;
    _activeErrorRecoveryScopes ??= [];
    _activeErrorRecoveryScopes.Add(this);
  }

  public static ErrorRecoveryScope? TryToRecover(Token? previousToken, Token nextToken)
  {
    if (_activeErrorRecoveryScopes == null)
    {
      return null;
    }

    // Find the top-most scope which can recover given the current tokens
    for (var i = _activeErrorRecoveryScopes.Count - 1; i >= 0; i--)
    {
      var scope = _activeErrorRecoveryScopes[i];
      if (scope._canRecover(previousToken, nextToken))
      {
        return scope;
      }

      if (scope._behavior == ErrorRecoveryBehavior.BlockOuterScopeRecovery && nextToken.TokenType != TokenType.EndOfFile)
      {
        // This scope blocks searching in outer scopes. This is to avoid the following situation:
        // <scope A>
        // { <scope B>
        //   <scope C> foo <-- error here
        //   bar <-- scope A detects recovery here
        // } <-- scope B should have instead detected recovery here
        // Note that we don't block if we encounter EndOfFile - that should always fall back to the outer-most scope
        return null;
      }
    }

    return null;
  }

  public void Dispose()
  {
    if (!_disposed)
    {
      Debug.Assert(_activeErrorRecoveryScopes != null);
      Debug.Assert(_activeErrorRecoveryScopes[^1] == this);
      _activeErrorRecoveryScopes.RemoveAt(_activeErrorRecoveryScopes.Count - 1);
      _disposed = true;
    }

    GC.SuppressFinalize(this);
  }
}

file class ErrorRecoveryException(ErrorRecoveryScope errorRecoveryScope)
  : Exception("Recovering from parser error")
{
  public ErrorRecoveryScope ErrorRecoveryScope => errorRecoveryScope;
}