var buf = '';
for(var i = 32; i < 127; i++)
  buf += String.fromCharCode(i) + ', ';
console.log(buf);
