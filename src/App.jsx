import React, { useEffect,useState} from 'react';
import Square from './Square';
import bb from "./assets/bb.svg";
import kb from "./assets/kb.svg";
import nb from "./assets/nb.svg";
import qb from "./assets/qb.svg";
import rb from "./assets/rb.svg";
import pb from "./assets/pb.svg";
import bw from "./assets/bw.svg";
import kw from "./assets/kw.svg";
import nw from "./assets/nw.svg";
import qw from "./assets/qw.svg";
import rw from "./assets/rw.svg";
import pw from "./assets/pw.svg";

function App(){
    const [response,setResponse]=useState("Waiting...")
    const [bitboard,setBitboard]=useState(-1);
    const [currentBitboard, setCurrentBitboard]=useState(0);
    const reference={
        "bb":bb,
        "kb":kb,
        "nb":nb,
        "qb":qb,
        "rb":rb,
        "pb":pb,
        "bw":bw,
        "kw":kw,
        "nw":nw,
        "qw":qw,
        "rw":rw,
        "pw":pw
    }
    useEffect(() => {
        fetchDataW((data)=>{
            // console.log("FIRST TRY BABY!")
            setResponse(JSON.parse(data));
        },"fenArr");
    }, []);

    async function fetchDataW(func,dataPoint) {
        try {
            const response = await fetch('http://localhost:8080/'+dataPoint);
            const data = await response.text();
            console.log('Response from C++ server:', JSON.parse(data)); 
            func(data);
        } catch (error) {
            console.error('Error fetching data:', error);
        }
    };

    let squaresEls=[];
    if(response!="Waiting..."){
        for(let i=0; i<64;i++){
            let y=Math.floor(i/8);
            let x=i%8;
            let pieceChar=response[y][x]
            // console.log((bitboard!=-1 && bitboard[currentBitboard][y][x]));
            squaresEls.push(
                <Square 
                    tileColor={(x+y)%2}
                    piece={pieceChar}
                    pieceSvg={reference[pieceChar.toLowerCase() + ((pieceChar==pieceChar.toUpperCase())?"b":"w")]}
                    colored={(bitboard!=-1 && bitboard[currentBitboard][y][x])}
                    color="#FF0000"
                />
            )
        }
    }
    let buttonEls=[];
    for(let i=0;i<bitboard.length;i++){
        buttonEls.push(<>
            <button className='changeBitboardButtons'
                onClick={()=>setCurrentBitboard(i)}
            >{i+1}</button>
        </>)
    }
    

    return (
        <div>
        <h1>React App</h1>
        <p>{response}</p>
        <div className='tiles'>
            {squaresEls}
        </div>
        <button className='bitboardButton'
            onClick={()=>fetchDataW((data)=>{setBitboard(JSON.parse(data))},"getBitboards")}
        >{bitboard==-1? "Get":"Refresh"} bitboard</button>
        {buttonEls}
        </div>
    );
};

export default App;