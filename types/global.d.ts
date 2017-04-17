/** Global definitions for developement **/

// for style loader
declare module '*.css' {
  const styles: any;
  export = styles;
}

// for mips/merl loader
declare module '*.merl' {
  const array: Uint32Array;
  export default array;
}
declare module '*.mips' {
  const array: Uint32Array;
  export default array;
}

// for asm loader
declare module '*.asm' {
  const code: string;
  export default code;
}

// for redux devtools extension
declare interface Window {
  Module?: any;
}
