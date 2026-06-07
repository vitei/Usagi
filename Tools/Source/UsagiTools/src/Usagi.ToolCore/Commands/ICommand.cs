namespace Usagi.ToolCore.Commands;

public interface ICommand
{
    string Description { get; }
    void Execute();
    void Undo();
}
