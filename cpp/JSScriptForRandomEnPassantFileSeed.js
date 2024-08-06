stringThingy = "{"
for(let j=0; j<8;j++){
  stringThingy+="{0b"
  for(let bit=0; bit<64; bit++){
    if(Math.random()>0.5){
      stringThingy+="1"
    }else{
      stringThingy+="0"
    }
  }
  stringThingy+="}"
  if(j!=7){
    stringThingy+=","
  }
}
stringThingy+="}"
console.log(stringThingy)