import React, { Component } from 'react';

import './App.css';

import brace from 'brace'; // eslint-disable-line
import AceEditor from 'react-ace';

import './mips_ace_syntax';
import 'brace/theme/monokai';

import 'brace/ext/searchbox';

import sample from './samples/recur_proc.asm';

class App extends Component {
  state = {
    code: sample,
    log: [],
    annotations: [],
    $1: 4919,
    $2: 448453205,
  }

  run = () => {
    this.setState({ log: [] });

    // The following code is bad, AND I should feel bad.
    if (window.Module) {
      try {
        window.Module.exit(1);
      } catch (e) {
      } finally {
        window.Module = { monitorRunDependencies: false };
      }
    }

    // reload the ol' scriptaroo
    const oldTag = document.getElementById("emscriptenCode");
    if (oldTag) oldTag.remove();

    const sc = document.createElement("script");
    sc.setAttribute("id", "emscriptenCode");
    sc.setAttribute("src", "mips241.js");
    sc.setAttribute("type", "text/javascript");
    document.head.appendChild(sc);

    const self = this;
    (function gross() {
      if (!window.Module || !window.Module.MIPS$$RAM) {
        setTimeout(gross, 100);
        return;
      }

      // just until I implement a native debug view
      window.Module.printErr = x => { self.setState({ log: self.state.log.concat(x) }); };

      const ram = new window.Module.MIPS$$RAM();
      const bus = new window.Module.MIPS$$BUS(ram);
      const cpu = new window.Module.MIPS$$CPU(bus);

      const debug = new window.Module.MIPS$$Debugger(cpu, ram, bus);

      let words;
      try {
        words = window.Module.MIPS$$asm(self.state.code)
          .split(" ")
          .filter((x,i,a) => i !== a.length - 1)
          // .map(x => { console.log(x); return x; })
          .map(x => parseInt(x, 10));
      } catch (err) {
        console.log(err);

        if (typeof err === "number") {
          self.setState({
            annotations: [{
              row: 0,
              column: 0,
              type: 'error',
              text: "Unknown error, could not assemble ¯\\_(ツ)_/¯"
            }]
          });
        } else {
          // This error handling matches the errors thrown by my assembler.
          // Your milage will vary.

          // If the errors are the ones I wrote, they have line numbers. Life
          // becomes easy.
          const m = err.match(/line (\d+)/);

          if (m && m[1]) {
            self.setState({
              log: [err],
              annotations: [{
                row: parseInt(m[1], 10) - 1,
                column: 0,
                type: 'error',
                text: err
              }]
            });
            return;
          }

          // If all else fails...
          self.setState({
            log: [err],
            annotations: [{
              row: 0,
              column: 0,
              type: 'error',
              text: err
            }]
          });
        }
        return;
      }

      words.forEach((x, i) => { ram.store(i * 4, x); });

      cpu.setRegister(1, self.state.$1 >>> 0);
      cpu.setRegister(2, self.state.$2 >>> 0);

      // debug.printRAMFrom(0, 30);

      let outStr = "";
      while (cpu.stillExecuting()) {
        for (var i = 0; i < 5; i++) {
          cpu.do_cycle();
        }

        outStr += bus.getOutput();
      }

      self.setState({ log: self.state.log.concat(outStr), annotations: [] });

      // console.log(`
      //   Stack RAM
      //   ---------
      // `);
      // debug.addhiglight(cpu.getRegister(30));
      // debug.printRAMFrom(cpu.getRegister(30) - 10 * 4, 12);
      // debug.removehiglight(cpu.getRegister(30));

      // console.log(`
      //   Program RAM
      //   -----------
      // `);
      // debug.addhiglight(cpu.getiRegister("PC"));
      // debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
      // debug.removehiglight(cpu.getiRegister("PC"));
      debug.printCPUState();

      ram.delete(); // nontrivial destructor
    })();
  }

  render() {
    return (
      <div className="App">
        <AceEditor
          annotations={this.state.annotations}
          width=""
          height=""
          mode="mips_assembler"
          theme="monokai"
          onChange={code => this.setState({code})}
          name="codeeditor"
          editorProps={{$blockScrolling: true}}
          deafultValue={this.state.code}
          value={this.state.code}
        />
        <div className="output">
          <code>
            {this.state.log.map((s, i) => <span key={i}>{s.replace(/ /g, "\u00a0")}<br/></span>)}
          </code>
        </div>
        <div className="run" onClick={this.run}>Run</div>

        <div className="input">
          <label>
            $1
            <input
              onChange={e => this.setState({ $1: e.target.value })}
              value={this.state.$1}
              type="number"
              name="$1"
              min={-2147483648}
              max={2147483647}
            />
          </label>
          <label>
            $2
            <input
              onChange={e => this.setState({ $2: e.target.value })}
              value={this.state.$2}
              type="number"
              name="$2"
              min={-2147483648}
              max={2147483647}
            />
          </label>
        </div>
      </div>
    );
  }
}

export default App;
