// NOT USED, WIP
import * as React from 'react';

import * as _ from 'lodash';

import { CPUProxy } from '~/libs/mips241web';

import './CPU.css';

interface CPUViewProps {
  cpu: CPUProxy | undefined;
}

export default ({ cpu }: CPUViewProps) => {
  if (typeof cpu === "undefined") {
    return <div className="cpu-view">Nothing Yet</div>
  }

  const registers = _.reduce(
    Object.keys(cpu),
    (a, register) => {
      if (register.startsWith("R") || register.startsWith("I")) { return a; }
      const value = cpu[register];
      return [...a, (
        <div key={register} className="cpu-view__register">
          {_.pad(register, 2)}: {"0x" + _.padStart(value.toString(16), 8, '0')}
        </div>
      )];
    },
    Array()
  );

  return (
    <div className="cpu-view">
      {registers}
    </div>
  );
}
