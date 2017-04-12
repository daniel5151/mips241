import React, { Component } from 'react';

import brace from 'brace'; // eslint-disable-line
import AceEditor from 'react-ace';

import 'brace/theme/monokai';
import '~/syntax/mips241';

import 'brace/ext/searchbox';

import _ from 'lodash';

// let's NOT touch spaghetti code
import { getEmu, assemble } from '~/libs/mips241web';
import { printMIPS } from '~/libs/getMIPSfile';

import printCode from '~/libs/print.json';
const PRINTADDR = 0xFFFF8000;

import sample from '~/asm_samples/recur_proc.asm';

import './App.css';

class App extends Component {
  state = {
    code: sample,
    assembled: [],

    log: [],
    annotations: [],

    $1: 4919,
    $2: 448453205,
  }

  assemble = async () => {
    try {
      const words = await assemble(this.state.code);

      console.log(printMIPS(words));

      this.setState({
        assembled: words,
        annotations: []
      });
      return true;
    } catch (err) {
      this.setState({
        log: this.state.log.concat(err.text),
        annotations: [err]
      });
      return false;
    }
  }

  run = async () => {
    this.setState({ log: [] });

    const didAssemble = await this.assemble();
    if (!didAssemble) return;

    const { ram, bus, cpu, debug, proxy } = await getEmu({
      // onSTDOUT: () => {},
      onSTDERR: (s) => this.setState({ log: this.state.log.concat(s) })
    });

    // inject program code
    _.forEach(this.state.assembled, (x, i) => ram.store(i * 4, x));
    // inject print code
    _.forEach(printCode, (x, i) => ram.store(PRINTADDR + i * 4, x));

    // Set registers
    cpu.setRegister(1, this.state.$1 >>> 0);
    cpu.setRegister(2, this.state.$2 >>> 0);

    // Main execution!
    let outStr = "";
    while (cpu.stillExecuting()) {
      for (var i = 0; i < 5; i++) {
        cpu.do_cycle();
      }

      outStr += bus.getOutput();
    }

    this.setState({
      log: this.state.log.concat(outStr),
      annotations: []
    });

    debug.addhiglight(cpu.getiRegister("PC"));
    debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
    debug.removehiglight(cpu.getiRegister("PC"));
    debug.printCPUState();

    ram.delete(); // nontrivial destructor
  }

  assemble_db = _.debounce(this.assemble, 500)
  handleCodeChange = (code) => {
    this.setState({ code });
    this.assemble_db(code);
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
          onChange={this.handleCodeChange}
          name="codeeditor"
          editorProps={{$blockScrolling: true}}
          deafultValue={this.state.code}
          value={this.state.code}
        />
        <div className="output">
          <code>
            {this.state.log.map((s, i) => (
              <span key={i}>{s.replace(/ /g, "\u00a0")}<br/></span>
            ))}
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
