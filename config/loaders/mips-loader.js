module.exports = function(content) {
  this.cacheable && this.cacheable();
  this.value = content;

  var array = new Array();
  for (var i = 0; i != content.length; ++i) {
    array[i] = content[i];
  }

  var reverseEndian = new Array();
  for (let i = 0; i < array.length; i += 4) {
    reverseEndian.push(array[i + 3], array[i + 2], array[i + 1], array[i])
  }

  const uint8array = new Uint8Array(reverseEndian);
  const uint32array = new Uint32Array(uint8array.buffer);
  const finalArray = [...uint32array];

  return 'module.exports = new Uint32Array(' + JSON.stringify(finalArray) + ')';
};

module.exports.raw = true;
