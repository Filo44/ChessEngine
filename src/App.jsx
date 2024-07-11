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
    const [responded, setResonded]= useState(false);
    const [posMoves,setPosMoves]=useState(-1);
    const [currPieceType, setCurrPieceType]=useState(null);
    const [capOrMove, setCapOrMove] = useState(-1);
    const [currPiece, setCurrPiece]=useState(-1);
    const [bitboardVisibility,setbitboardVisibility]=useState(true);
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
    const pieceCharToFullPieceName = {
        "r":"Rook",
        "n":"Knight",
        "b":"Bishop",
        "k":"King", 
        "q":"Queen",
        "p":"Pawn"
    }

    useEffect(() => {
        fetchDataW((data)=>{
            // console.log("FIRST TRY BABY!")
            setResponse(JSON.parse(data));
        },"data");
    }, []);
    async function fetchDataW(func,dataPoint) {
        try {
            const response = await fetch('http://localhost:8080/'+dataPoint);
            const data = await response.text();
            console.log(data);
            console.log('Response from C++ server:', JSON.parse(data)); 
            func(data);
            setResonded(true)
        } catch (error) {
            console.log('Error fetching data:', error);
            setResponse("Failed to fetch board data, trying again...")
            setTimeout(fetchDataW(func, dataPoint),1000)
        }
    };

    //*Stupid, for some reason it goes bottom to top. Going to go backwards to reverse it.
    let squaresEls=[];
    if(responded){
        for(let i=0; i<64;i++){
            let y=Math.floor(i/8);
            let x=i%8;
            // console.log("x:",x,"y:",y)
            let pieceChar=response[y][x]
            console.log(response)
            console.log(responded)
            // console.log((bitboard!=-1 && bitboard[currentBitboard][y][x]));
            let colored=false;
            if(posMoves!=-1 && currPieceType!=-1 && currPiece!=null && capOrMove!=-1){
                let currPieceTypeType=posMoves.pieceTypes[currPieceType];
                console.log("Currpiecetype:" + currPieceType)
                if(currPiece==-1){
                    //Means it is looking at the combined BB
                    colored=capOrMove==0 ? currPieceTypeType.combinedMoveBB[y][x]:currPieceTypeType.combinedCapBB[y][x];
                }else{
                    console.log("currPieceTypeType:" + JSON.stringify(currPieceTypeType));
                    console.log("currPieceTypeType.pieces:" + JSON.stringify(currPieceTypeType.pieces));
                    console.log("currPiece:" + JSON.stringify(currPiece));
                    console.log("currPieceTypeType.pieces[currPiece]:" + JSON.stringify(currPieceTypeType.pieces[currPiece]));
                    if(capOrMove==0){
                        colored=currPieceTypeType.pieces[currPiece].moveBitboard[y][x];
                    }else{
                        colored=currPieceTypeType.pieces[currPiece].capBitboard[y][x];
                    }
                }

            }
            
            squaresEls.push(
                <Square 
                    tileColor={(x+y)%2}
                    piece={pieceChar}
                    pieceSvg={reference[pieceChar.toLowerCase() + ((pieceChar==pieceChar.toUpperCase())?"w":"b")]}
                    colored={colored}
                    color="#FF0000"
                />
            )
        }
    }
    let pieceTypeButtons=[];
    let piecesButtons=[];

    if(posMoves!=-1){
        for(let i=0;i<posMoves.pieceTypes.length;i++){
            let pieceType = posMoves.pieceTypes[i];
            let pieceTypeChar = pieceType.pieceChar;
            if(pieceType.pieces.length==0){
                continue;
            }
            pieceTypeButtons.push(<>
                <button className={'pieceButtons' + (currPieceType==i ? " greyed": "")}
                    onClick={()=>{setCurrPieceType(i);setCurrPiece(-1);}}
                >{pieceCharToFullPieceName[pieceTypeChar]}</button>
        </>)
        }
        if(currPieceType!=-1){
            piecesButtons.push(
                <>
                    <button className={'pieceButtons' + (capOrMove==0 ? " greyed": "")}
                        onClick={()=>{setCapOrMove(0)}}
                    >
                        Move Bitboards
                    </button>
                </> 
            )
            piecesButtons.push(
                <>
                    <button className={'pieceButtons' + (capOrMove==1 ? " greyed": "")}
                        onClick={()=>{setCapOrMove(1)}}
                    >
                        Capture Bitboards
                    </button>
                </>
                    
            )
            piecesButtons.push(<>
                <br/>
            </>)
            if(capOrMove!=-1){
                if(posMoves.pieceTypes[currPieceType].pieces.length!=0){
                    piecesButtons.push(<>
                        <button className={'pieceButtons' + (currPiece==-1 ? " greyed": "")}
                            onClick={()=>setCurrPiece(-1)}
                            >{capOrMove==0 ? "Piece Move Combined Bitboard" : "Piece Capture Combined Bitboard"}</button>
                    </>)
                }

                piecesButtons.push(<>
                    <br/>
                </>);

                for(let i=0;i<posMoves.pieceTypes[currPieceType].pieces.length;i++){
                    let piece = posMoves.pieceTypes[currPieceType].pieces[i];
                    piecesButtons.push(<>
                        <button className={'pieceButtons' + (currPiece==i ? " greyed": "")}
                            onClick={()=>{setCurrPiece(i)}}
                            >{i+1}</button>
                    </>)
                }
            }
            
        }
        
    }
    
    
    return (
        <div>
        <h1>The ACE</h1>
        <p>{response}</p>
        <div className='tiles'>
            {squaresEls}
        </div>
        <button className='bitboardButton'
            onClick={()=>fetchDataW((data)=>{setPosMoves(JSON.parse(data))},"getBitboards")}
        >{posMoves==-1? "Get":"Refresh"} bitboard</button>
        {posMoves!=-1 && <button className='bitboardButton'
            onClick={()=>setbitboardVisibility((visibility)=>!visibility)}
        >Turn bitboards {bitboardVisibility? "Off":"On"}</button>}
        {pieceTypeButtons}
        <br/>
        {currPieceType!= -1 && piecesButtons}
        </div>
    );
};

export default App;