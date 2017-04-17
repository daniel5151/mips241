import { flow, map, padCharsStart, chunk, join, flatten } from 'lodash/fp';

// const print = x => (console.log(x), x);

export function printMIPS(words: Array<number>) {
  return flow(
    map(flow(
      (x: number) => x.toString(16),
      padCharsStart("0")(8),
      chunk(4),
      map(join("")),
    )),
    flatten,
    // at this point, we have a array of 16bit hex nums
    chunk(8),
    map(join(" ")),
    join("\n"),
  )(words);
}
