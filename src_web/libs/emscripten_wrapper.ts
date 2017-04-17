// For some reason, that I really did not want to look into, I
// a) couldn't get emscripten to play nice with webpack
// b) couldn't figure out why repeated call to my assembler function failed

// This is my ghetto solution. Reinit the entire c++ program on each call.

// The following code is bad, AND I feel bad. Please no judge :(

function getCPP(): Promise<Mips241> {
  return new Promise(resolve => {
    // Kill any running c++
    if (window.Module) {
      try {
        window.Module.exit(1);
      } catch (e) {
        // dun worry bout it fam ;)
      } finally {
        // why? dunno, but it prevents errors ;)
        window.Module = { monitorRunDependencies: false };
      }
    }

    // reload the ol' scriptaroo
    const oldTag = document.getElementById("emscriptendCode");
    if (oldTag) { oldTag.remove(); };

    const sc = document.createElement("script");
    sc.setAttribute("id", "emscriptendCode");
    sc.setAttribute("src", "mips241.js");
    sc.setAttribute("type", "text/javascript");
    document.head.appendChild(sc);

    // ...and poll for when it's initialized
    // (inb4 "you shouldn't be allowed to touch a computer")
    (function neverdothis() {
      if (!window.Module || !window.Module.MIPS$$RAM) {
        setTimeout(neverdothis, 100);
        return;
      } else {
        console.groupEnd();
        resolve(window.Module);
      }
    })();
  });
}

import { throttle } from 'lodash';
export default throttle(getCPP, 250);
