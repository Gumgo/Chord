using Compiler.Ast;
using Compiler.Utilities;
using System.Diagnostics;

namespace Compiler.AstBuilding;

internal enum ScopeType
{
  Normal,
  Conditional,
  ForLoop,
}

internal enum ScopeTrackerState
{
  No,
  Maybe,
  Yes,
}

internal class ScopeTracker
{
  private readonly ScopeAstNode _globalScope;
  private readonly ScopeTracker? _parent;
  private readonly Dictionary<ValueDefinitionAstNode, ScopeTrackerState> _trackedValueInitializedStates = [];

  public ScopeTracker(ScopeAstNode globalScope)
  {
    _globalScope = globalScope;
    ScopeType = ScopeType.Normal;
  }

  public ScopeTracker(ScopeTracker parent, ScopeType scopeType)
  {
    _globalScope = parent._globalScope;
    _parent = parent;
    foreach (var (key, value) in parent._trackedValueInitializedStates)
    {
      _trackedValueInitializedStates.Add(key, value);
    }

    ScopeType = scopeType;
    InForLoop = scopeType == ScopeType.ForLoop || parent.InForLoop;

    DidBreak = parent.DidBreak;
    DidContinue = parent.DidContinue;
    DidReturn = parent.DidReturn;
    IsUnreachable = parent.IsUnreachable;
  }

  private ScopeTracker(ScopeTracker copyFrom)
  {
    _globalScope = copyFrom._globalScope;
    _parent = copyFrom._parent;
    foreach (var (key, value) in copyFrom._trackedValueInitializedStates)
    {
      _trackedValueInitializedStates.Add(key, value);
    }

    ScopeType = copyFrom.ScopeType;
    InForLoop = copyFrom.InForLoop;

    DidBreak = copyFrom.DidBreak;
    DidContinue = copyFrom.DidContinue;
    DidReturn = copyFrom.DidReturn;
    IsUnreachable = copyFrom.IsUnreachable;
  }

  private ScopeTracker(ScopeAstNode globalScope, ScopeTracker? parent, ScopeType scopeType)
  {
    _globalScope = globalScope;
    _parent = parent;

    ScopeType = scopeType;
    InForLoop = scopeType == ScopeType.ForLoop || (parent?.InForLoop ?? false);
  }

  public ScopeType ScopeType { get; }
  public bool InForLoop { get; }

  public ScopeTrackerState DidBreak { get; private set; } = ScopeTrackerState.No;
  public ScopeTrackerState DidContinue { get; private set; } = ScopeTrackerState.No;
  public ScopeTrackerState DidReturn { get; private set; } = ScopeTrackerState.No;
  public bool IsUnreachable { get; private set; }

  public static ScopeTracker Union(IReadOnlyList<ScopeTracker> scopeTrackers)
  {
    Debug.Assert(!scopeTrackers.IsEmpty());
    Debug.Assert(scopeTrackers.All((scopeTracker) => scopeTracker._parent == scopeTrackers[0]._parent));
    Debug.Assert(scopeTrackers.All((scopeTracker) => scopeTracker.ScopeType == scopeTrackers[0].ScopeType));

    var scopeTracker = new ScopeTracker(scopeTrackers[0]._globalScope, scopeTrackers[0]._parent, scopeTrackers[0].ScopeType);

    // If a scope breaks, continues, or returns, the containing scope won't be entered
    var filteredScopeTrackers = scopeTrackers.Where((v) => !v.IsUnreachable).ToArray();

    var allTrackedValues = filteredScopeTrackers.SelectMany((scopeTracker) => scopeTracker._trackedValueInitializedStates.Keys).ToHashSet();
    foreach (var valueDefinition in allTrackedValues)
    {
      var initializedStates = filteredScopeTrackers
        .Select((scopeTracker) => scopeTracker._trackedValueInitializedStates.GetValueOrDefault(valueDefinition, ScopeTrackerState.No))
        .ToArray();
      scopeTracker._trackedValueInitializedStates[valueDefinition] = UnionConditionalScopeStates(initializedStates);
    }

    scopeTracker.DidBreak = UnionConditionalScopeStates(scopeTrackers.Select((tracker) => tracker.DidBreak).ToArray());
    scopeTracker.DidContinue = UnionConditionalScopeStates(scopeTrackers.Select((tracker) => tracker.DidContinue).ToArray());
    scopeTracker.DidReturn = UnionConditionalScopeStates(scopeTrackers.Select((tracker) => tracker.DidReturn).ToArray());
    scopeTracker.IsUnreachable = scopeTrackers.All((tracker) => tracker.IsUnreachable);

    return scopeTracker;
  }

  public ScopeTracker Clone()
    => new(this);

  public void TrackValue(ValueDefinitionAstNode valueDefinition, ScopeTrackerState initializedState)
    => _trackedValueInitializedStates.Add(valueDefinition, initializedState);

  public ScopeTrackerState GetValueInitializedState(ValueDefinitionAstNode valueDefinition)
  {
    // Globals are assumed to always be initialized
    if (_globalScope.ScopeItems.Contains(valueDefinition))
    {
      return ScopeTrackerState.Yes;
    }

    if (IsUnreachable)
    {
      // If this code is unreachable, we can't determine whether values have been initialized (because we can't trace a path to an unreachable location in the
      // code) so treat everything as if it's been initialized - this code won't execute so it doesn't actually matter.
      return ScopeTrackerState.Yes;
    }

    var scopeTracker = this;
    while (true)
    {
      if (scopeTracker._trackedValueInitializedStates.TryGetValue(valueDefinition, out var state))
      {
        return state;
      }

      scopeTracker = scopeTracker._parent ?? throw new ArgumentException("Value is not tracked");
    }
  }

  public void Assign(ValueDefinitionAstNode valueDefinition)
  {
    if (_globalScope.ScopeItems.Contains(valueDefinition))
    {
      // It is an error to try to assign to a global value outside of its initializer so we'll just ignore the assignment if it occurs (errors are reported
      // outside of this code)
      return;
    }

    if (IsUnreachable)
    {
      // If this code is unreachable, we're not actually going to make the assignment so don't update initialization state
      return;
    }

    _trackedValueInitializedStates[valueDefinition] = ScopeTrackerState.Yes;
  }

  public void IssueBreak()
  {
    Debug.Assert(InForLoop);

    if (IsUnreachable)
    {
      // If this code is unreachable, we're not actually going to make the assignment so don't update state
      return;
    }

    DidBreak = ScopeTrackerState.Yes;
    IsUnreachable = true;
  }

  public void IssueContinue()
  {
    Debug.Assert(InForLoop);

    if (IsUnreachable)
    {
      // If this code is unreachable, we're not actually going to make the assignment so don't update state
      return;
    }

    DidContinue = ScopeTrackerState.Yes;
    IsUnreachable = true;
  }

  public void IssueReturn()
  {
    if (IsUnreachable)
    {
      // If this code is unreachable, we're not actually going to make the assignment so don't update state
      return;
    }

    DidReturn = ScopeTrackerState.Yes;
    IsUnreachable = true;
  }

  public void IntegrateChildScope(ScopeTracker childScopeTracker)
  {
    foreach (var valueDefinition in _trackedValueInitializedStates.Keys.ToArray())
    {
      if (childScopeTracker._trackedValueInitializedStates.TryGetValue(valueDefinition, out var childState))
      {
        _trackedValueInitializedStates[valueDefinition] = Max(_trackedValueInitializedStates[valueDefinition], childState);
      }
    }

    // DidBreak and DidContinue state don't propagate past the for loop scope
    if (childScopeTracker.ScopeType != ScopeType.ForLoop)
    {
      DidBreak = Max(DidBreak, childScopeTracker.DidBreak);
      DidContinue = Max(DidContinue, childScopeTracker.DidContinue);
    }

    DidReturn = Max(DidReturn, childScopeTracker.DidReturn);
    IsUnreachable |= childScopeTracker.IsUnreachable;
  }

  private static ScopeTrackerState UnionConditionalScopeStates(IReadOnlyList<ScopeTrackerState> scopeStates)
  {
    var yesInAllScopes = scopeStates.All((v) => v == ScopeTrackerState.Yes);
    var noInAllScopes = scopeStates.All((v) => v == ScopeTrackerState.No);
    return yesInAllScopes
      ? ScopeTrackerState.Yes
      : (noInAllScopes ? ScopeTrackerState.No : ScopeTrackerState.Maybe);
  }

  private static ScopeTrackerState Max(ScopeTrackerState a, ScopeTrackerState b)
    => (ScopeTrackerState)Math.Max((int)a, (int)b);
}