using System.Diagnostics;
using System.Reflection;
using System.Text;

namespace Compiler.Lexer;

internal interface ISymbolTokenDetectorState
{
  TokenType? TokenType { get; }
  bool Rejected { get; }
}

// This is a mini DFA used to detect symbols (exact matches only, not general regex)
internal class SymbolTokenDetector
{
  // These symbols need to be separated from each other. For example, &&|| is disallowed and should be immediately flagged as an invalid token rather than
  // getting caught later as invalid syntax.
  private static readonly HashSet<TokenType> _symbolsDisallowingAdjacency =
    [
      TokenType.Assign,
      TokenType.AssignPlus,
      TokenType.AssignMinus,
      TokenType.AssignMultiply,
      TokenType.AssignDivide,
      TokenType.AssignModulo,
      TokenType.AssignAnd,
      TokenType.AssignOr,
      TokenType.LogicalOr,
      TokenType.LogicalAnd,
      TokenType.BitwiseOr,
      TokenType.BitwiseXor,
      TokenType.BitwiseAnd,
      TokenType.Equal,
      TokenType.NotEqual,
      TokenType.LessThan,
      TokenType.GreaterThan,
      TokenType.LessThanEqual,
      TokenType.GreaterThanEqual,
      TokenType.Plus,
      TokenType.Minus,
      TokenType.Multiply,
      TokenType.Divide,
      TokenType.Modulo,
      TokenType.Not,
      TokenType.NotAlternate,
    ];

  private static readonly HashSet<Rune> _symbolsDisallowingAdjacencyCharacters = new(
    _symbolsDisallowingAdjacency.SelectMany(
      (tokenType) =>
      {
        var symbolTokenAttribute = typeof(TokenType).GetField(tokenType.ToString())?.GetCustomAttribute<SymbolTokenAttribute>()
          ?? throw new Exception("Token type is not a symbol");
        return symbolTokenAttribute.Text.EnumerateRunes();
      }));

  // The starting state
  private readonly State _rootState = new();

  // If there are no more possible symbols to match, we'll move into this state
  private readonly State _invalidState = new();

  // If we've matched a symbol in the disallow-adjacency list and then match a rune from one of the disallow-adjacency symbols, we'll move into this state. Once
  // we're here, we'll keep reading any disallow-adjacency symbols that arrive (so that we can read the entire invalid token rather than just the first rune)
  // and then move into the invalid state.
  private readonly State _rejectedState = new() { Rejected = true };

  public SymbolTokenDetector()
  {
    foreach (var tokenType in Enum.GetValues<TokenType>())
    {
      var symbolTokenAttribute = typeof(TokenType).GetField(tokenType.ToString())?.GetCustomAttribute<SymbolTokenAttribute>();
      if (symbolTokenAttribute == null)
      {
        continue;
      }

      var currentState = _rootState;
      foreach (var rune in symbolTokenAttribute.Text.EnumerateRunes())
      {
        if (!currentState.Transitions.TryGetValue(rune, out var newState))
        {
          newState = new State();
          currentState.Transitions.Add(rune, newState);
        }

        currentState = newState;
      }

      // If this fires, it means we have a conflict (two identical symbol strings)
      Debug.Assert(currentState.TokenType == null);
      currentState.TokenType = tokenType;
    }
  }

  // Attempts to advance to the next state, returning true if successful
  public bool Advance(Rune rune, ref ISymbolTokenDetectorState? currentState)
  {
    var canReject = currentState == _rejectedState || (currentState?.TokenType != null && _symbolsDisallowingAdjacency.Contains(currentState.TokenType.Value));
    var newState = ((State?)currentState ?? _rootState).Transitions.GetValueOrDefault(rune, _invalidState);
    if (newState == _invalidState && canReject && _symbolsDisallowingAdjacencyCharacters.Contains(rune))
    {
      newState = _rejectedState;
    }

    currentState = newState;
    return currentState != _invalidState;
  }

  private class State : ISymbolTokenDetectorState
  {
    public TokenType? TokenType { get; set; }
    public bool Rejected { get; set; }
    public Dictionary<Rune, State> Transitions { get; } = [];
  }
}