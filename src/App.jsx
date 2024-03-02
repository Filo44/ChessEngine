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
        fetchData();
    }, []);

    const fetchData = async () => {
        try {
            const response = await fetch('http://localhost:8080/fenArr');
            const data = await response.text();
            console.log('Response from C++ server:', JSON.parse(data)); 
            setResponse(JSON.parse(data));
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
            squaresEls.push(
                <Square 
                    tileColor={(x+y)%2}
                    piece={pieceChar}
                    pieceSvg={reference[pieceChar.toLowerCase() + ((pieceChar==pieceChar.toUpperCase())?"b":"w")]}
                />
            )
        }
    }
    

    return (
        <div>
        <h1>React App</h1>
        <p>{response}</p>
        <div className='tiles'>
            {squaresEls}
        </div>
        </div>
    );
};

export default App;