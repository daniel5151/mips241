import React, { Component } from 'react';

import brace from 'brace'; // eslint-disable-line
import AceEditor from 'react-ace';

import 'brace/theme/monokai';
import '~/syntax/mips241';

import 'brace/ext/searchbox';

import debounce from '~/libs/debounce';

// let's NOT touch spaghetti code
import { getEmu, assemble } from '~/libs/mips241web';

import printCode from '~/libs/print.json';
const PRINTADDR = 0xFFFF8000;

import sample from '~/asm_samples/recur_proc.asm';

import './App.css';

// eslint-disable-next-line
function chunk(a,b){for(var c=[],d=0,e=a.length;d<e;)c.push(a.slice(d,d+=b));return c}
// eslint-disable-next-line
function leftPad(a,b,c){if(a+="",b-=a.length,b<=0)return a;if(c||0===c||(c=" "),c+=""," "===c&&b<10)return cache[b]+a;for(var d="";;){if(1&b&&(d+=c),b>>=1,!b)break;c+=c}return d+a}var cache=[""," ","  ","   ","    ","     ","      ","       ","        ","         "];

function callFunc(addr) {
  return `
    ; store return addr on stack
    sw      $31, -4($30)
    lis     $31
    .word   4
    sub     $30, $30, $31

    ; call print
    lis     $31
    .word   ${addr}
    jalr    $31

    ; restore return addr
    lis     $31
    .word   4
    add     $30, $30, $31
    lw      $31, -4($30)
  `;
}

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
    // preprocess

    const code = this.state.code.replace(/print! (\$\d+)/g, (match, p1, offset) => {
      const macro = [
        `add     $1, $0, ${p1}`,
        callFunc(PRINTADDR),
      ];

      if (p1 !== "$1") {
        macro.unshift(`
          ; store $1 register on stack
          sw      $1, -4($30)
          lis     $1
          .word   4
          sub     $30, $30, $1
        `);
        macro.push(`
          ; restore $1 register
          lis     $1
          .word   4
          add     $30, $30, $1
          lw      $1, -4($30)
        `);
      }

      return macro.join("\n");
    });

    try {
      const words = await assemble(code);
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

    console.log(
      chunk(this.state.assembled.map(x => chunk(leftPad(x.toString(16), 8, "0"), 4).join(" ")), 4).map(x => x.join(" ")).join("\n")
    );

    const { ram, bus, cpu, debug } = await getEmu({
      // onSTDOUT: () => {},
      onSTDERR: (s) => this.setState({ log: this.state.log.concat(s) })
    });

    this.state.assembled.forEach((x, i) => {
      ram.store(i * 4, x);
    });

    // inject print code
    printCode.forEach((x, i) => {
      ram.store(PRINTADDR + i * 4, x);
    });

    cpu.setRegister(1, this.state.$1 >>> 0);
    cpu.setRegister(2, this.state.$2 >>> 0);

    // debug.printRAMFrom(0, 30);

    let outStr = "";
    while (cpu.stillExecuting()) {
      for (var i = 0; i < 5; i++) {
        cpu.do_cycle();
      }

      outStr += bus.getOutput();

      // await new Promise(r => setTimeout(r, 1000));

      // debug.addhiglight(cpu.getiRegister("PC"));
      // debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
      // debug.removehiglight(cpu.getiRegister("PC"));
      // debug.printCPUState();
    }

    this.setState({ log: this.state.log.concat(outStr), annotations: [] });

    // log.append(`
    //   Stack RAM
    //   ---------
    // `);
    // debug.addhiglight(cpu.getRegister(30));
    // debug.printRAMFrom(cpu.getRegister(30) - 10 * 4, 12);
    // debug.removehiglight(cpu.getRegister(30));

    // log.append(`
    //   Program RAM
    //   -----------
    // `);
    debug.addhiglight(cpu.getiRegister("PC"));
    debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
    debug.removehiglight(cpu.getiRegister("PC"));
    debug.printCPUState();

    ram.delete(); // nontrivial destructor
  }

  assemble_db = debounce(this.assemble, 250)
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
