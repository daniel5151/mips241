import * as React from 'react';

import * as _ from 'lodash';

import 'brace'; // eslint-disable-line
import AceEditor, { Annotation } from 'react-ace';

import '~/syntax/mips241';
import 'brace/theme/monokai';
import 'brace/ext/searchbox';

// let's NOT touch spaghetti code
import { getEmu, assemble, CPUProxy } from '~/libs/mips241web';

import { printMIPS } from '~/libs/getMIPSfile';

// using raw-loader
const sample = require('~/asm_samples/recSum.asm');

// import CPUView from './CPU';

import './App.css';

type AppState = {
  code: string,
  assembled: number[],

  log: string[],
  annotations: Annotation[],

  array: number[],
  $1: number,
  $2: number,
};

class App extends React.Component<{}, AppState> {
  assembleDebounced: (() => Promise<boolean>) & _.Cancelable;

  _cpu: CPUProxy | undefined;

  constructor() {
    super();

    this.state = {
      code: sample,
      assembled: [],

      log: [],
      annotations: [],

      array: [],
      $1: 4919,
      $2: 448453205,
    };

    this.assembleDebounced = _.debounce(this.assemble, 500);
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
    if (!didAssemble) { return; }

    const { ram, bus, cpu, debug, proxy } = await getEmu({
      // onSTDOUT: () => {},
      onSTDERR: (s) => this.setState({ log: this.state.log.concat(s) })
    });

    this._cpu = proxy.cpu;

    // inject program code
    _.forEach(this.state.assembled, (x, i) => ram.store(i * 4, x));

    // Set registers
    cpu.setRegister(1, this.state.$1 >>> 0);
    cpu.setRegister(2, this.state.$2 >>> 0);

    // Main execution!
    let outStr = "";
    try {
      while (cpu.stillExecuting()) {
        for (var i = 0; i < 5; i++) {
          cpu.do_cycle();
        }

        outStr += bus.getOutput();
      }
    } catch (err) {
      this.setState({
        log: this.state.log.concat("ERROR: Invalid Opcode"),
        annotations: []
      });
    }

    // const cpuState = _.reduce(
    //   Object.keys(proxy.cpu),
    //   (a, k) => ({...a, [k]: proxy.cpu[k]}),
    //   {}
    // );

    // console.log(cpuState);

    this.setState({
      log: this.state.log.concat(outStr),
      annotations: []
    });

    // debug.addhiglight(cpu.getiRegister("PC"));
    // debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
    // debug.removehiglight(cpu.getiRegister("PC"));
    debug.printCPUState();

    ram.delete(); // nontrivial destructor
  }

  handleCodeChange = (code: string) => {
    this.setState({ code });
    this.assembleDebounced();
  }

  render() {
    const parseHex = (s: string): number => {
      return parseInt(s.replace(/0x/, ""), 16);
    };

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
          editorProps={{$blockScrolling: 1}}
          value={this.state.code}
        />

        {/*<CPUView cpu={this._cpu}/>*/}

        <div className="output">
          <code>
            {this.state.log
              .reduce((a, s, i) => {
                if (a.length === 0 && s === "") { return a; }
                return [...a, s];
              }, Array())
              .map((s, i) => (
                <span key={i}>{s.replace(/ /g, "\u00a0")}<br/></span>
              ))
            }
          </code>
        </div>

        <div className="control">
          <div className="run" onClick={this.run}>Run</div>

          <div className="register-in">
            <span>$1:</span>
            <input
              onChange={e => {
                if (e.target.value.match(/^(0x)?[0-9a-fA-F]{1,8}$/)) {
                  this.setState({ $1: parseHex(e.target.value) });
                }
              }}
              value={"0x" + this.state.$1.toString(16).toUpperCase()}
              type="text"
              min={-2147483648}
              max={2147483647}
            />
          </div>
          <div className="register-in">
            <span>$2:</span>
            <input
              onChange={e => {
                if (e.target.value.match(/^(0x)?[0-9a-fA-F]{1,8}$/)) {
                  this.setState({ $2: parseHex(e.target.value) });
                }
              }}
              value={"0x" + this.state.$2.toString(16).toUpperCase()}
              type="text"
              min={-2147483648}
              max={2147483647}
            />
          </div>
        </div>
      </div>
    );
  }
}

export default App;
