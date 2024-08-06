stringThingy = "{"
for(let i=0; i<12; i++){
  stringThingy+="{"
  for(let j=0; j<64;j++){
    stringThingy+="{0b"
    for(let bit=0; bit<64; bit++){
      if(Math.random()>0.5){
        stringThingy+="1"
      }else{
        stringThingy+="0"
      }
    }
    stringThingy+="}"
    if(j!=63){
      stringThingy+=","
    }
  }
  stringThingy+="}"
  if(i!=11){
    stringThingy+=","
  }
}
stringThingy+="}"
console.log(stringThingy)