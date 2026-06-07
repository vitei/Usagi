using Usagi.ToolCore.Commands;
using Xunit;

namespace Usagi.ToolCore.Tests;

public sealed class CommandHistoryTests
{
    [Fact]
    public void ExecuteAddsUndoEntryAndMarksDirty()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add item", state, "item"));

        Assert.True(history.IsDirty);
        Assert.True(history.CanUndo);
        Assert.False(history.CanRedo);
        Assert.Equal("add item", history.UndoDescription);
        Assert.Equal(["item"], state);
    }

    [Fact]
    public void UndoAndRedoMoveCommandBetweenStacks()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add item", state, "item"));
        history.Undo();

        Assert.Empty(state);
        Assert.False(history.CanUndo);
        Assert.True(history.CanRedo);
        Assert.Equal("add item", history.RedoDescription);

        history.Redo();

        Assert.Equal(["item"], state);
        Assert.True(history.CanUndo);
        Assert.False(history.CanRedo);
    }

    [Fact]
    public void ExecuteAfterUndoClearsRedoStack()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add first", state, "first"));
        history.Undo();
        history.Execute(new ListCommand("add second", state, "second"));

        Assert.False(history.CanRedo);
        Assert.Equal(["second"], state);
    }

    [Fact]
    public void MarkSavedTracksCurrentRevision()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add first", state, "first"));
        history.MarkSaved();
        history.Execute(new ListCommand("add second", state, "second"));

        Assert.True(history.IsDirty);

        history.Undo();

        Assert.False(history.IsDirty);
    }

    [Fact]
    public void BranchingFromSavedHistoryRemainsDirty()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add saved", state, "saved"));
        history.MarkSaved();
        history.Undo();
        history.Execute(new ListCommand("add different", state, "different"));

        Assert.True(history.IsDirty);
        Assert.Equal(["different"], state);
    }

    [Fact]
    public void ClearResetsHistoryAndDirtyState()
    {
        var state = new List<string>();
        var history = new CommandHistory();

        history.Execute(new ListCommand("add item", state, "item"));
        history.Clear();

        Assert.False(history.IsDirty);
        Assert.False(history.CanUndo);
        Assert.False(history.CanRedo);
    }

    [Fact]
    public void ChangedEventFiresForHistoryOperations()
    {
        var state = new List<string>();
        var history = new CommandHistory();
        var changeCount = 0;
        history.Changed += () => changeCount++;

        history.Execute(new ListCommand("add item", state, "item"));
        history.Undo();
        history.Redo();
        history.MarkSaved();
        history.Clear();

        Assert.Equal(5, changeCount);
    }

    private sealed class ListCommand(string description, List<string> state, string value) : ICommand
    {
        public string Description => description;

        public void Execute() => state.Add(value);

        public void Undo() => Assert.True(state.Remove(value));
    }
}
