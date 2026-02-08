const vscode = require('vscode');

function activate(context) {
  const command = vscode.commands.registerCommand('fluxbox.restart', async () => {
    const terminal = vscode.window.createTerminal('Restart Fluxbox');
    terminal.sendText('DISPLAY=:1 fluxbox -restart');
    terminal.show();
  });

  context.subscriptions.push(command);
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
