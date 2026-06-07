namespace Usagi.ToolCore.Commands;

public sealed class CommandHistory
{
    private readonly Stack<HistoryEntry> _undoStack = new();
    private readonly Stack<HistoryEntry> _redoStack = new();
    private long _currentRevision;
    private long _savedRevision;
    private long _nextRevision = 1;

    public event Action? Changed;

    public bool CanUndo => _undoStack.Count > 0;
    public bool CanRedo => _redoStack.Count > 0;
    public bool IsDirty => _currentRevision != _savedRevision;

    public string? UndoDescription => _undoStack.TryPeek(out var entry) ? entry.Command.Description : null;
    public string? RedoDescription => _redoStack.TryPeek(out var entry) ? entry.Command.Description : null;

    public void Execute(ICommand command)
    {
        var entry = new HistoryEntry(command, _currentRevision, _nextRevision++);
        command.Execute();
        _currentRevision = entry.AfterRevision;
        _undoStack.Push(entry);
        _redoStack.Clear();
        Changed?.Invoke();
    }

    public void Undo()
    {
        if (!CanUndo) return;

        var entry = _undoStack.Pop();
        entry.Command.Undo();
        _currentRevision = entry.BeforeRevision;
        _redoStack.Push(entry);
        Changed?.Invoke();
    }

    public void Redo()
    {
        if (!CanRedo) return;

        var entry = _redoStack.Pop();
        entry.Command.Execute();
        _currentRevision = entry.AfterRevision;
        _undoStack.Push(entry);
        Changed?.Invoke();
    }

    public void MarkSaved()
    {
        _savedRevision = _currentRevision;
        Changed?.Invoke();
    }

    public void Clear()
    {
        _undoStack.Clear();
        _redoStack.Clear();
        _currentRevision = 0;
        _savedRevision = 0;
        _nextRevision = 1;
        Changed?.Invoke();
    }

    private sealed record HistoryEntry(ICommand Command, long BeforeRevision, long AfterRevision);
}
