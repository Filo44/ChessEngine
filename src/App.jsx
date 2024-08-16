import React, { useEffect, useState } from "react";
import Square from "./Square";
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

const BLACK = false;
const WHITE = true;
const fileToX = {
    a:0,
    b:1,
    c:2,
    d:3,
    e:4,
    f:5,
    g:6,
    h:7
}

function App() {
    const [response, setResponse] = useState("Waiting...");
    const [responded, setResonded] = useState(false);
    const [posMoves, setPosMoves] = useState(-1);
    const [currPieceType, setCurrPieceType] = useState(null);
    const [capOrMove, setCapOrMove] = useState(-1);
    const [currPiece, setCurrPiece] = useState(-1);
    const [bitboardVisibility, setbitboardVisibility] = useState(true);
    const [evaluation, setEvaluation] = useState(0);
    const [gotEvaluation, setGotEvalution] = useState(false);
    const [isMoving, setIsMoving] = useState(false);
    const [moveInput, setMoveInput] = useState("");
    const [color, setColor] = useState(BLACK)
    const reference = {
        bb: bb,
        kb: kb,
        nb: nb,
        qb: qb,
        rb: rb,
        pb: pb,
        bw: bw,
        kw: kw,
        nw: nw,
        qw: qw,
        rw: rw,
        pw: pw,
    };
    const pieceCharToFullPieceName = {
        r: "Rook",
        n: "Knight",
        b: "Bishop",
        k: "King",
        q: "Queen",
        p: "Pawn",
    };
    const pieceToNumber = {
        'r': 0,
        'n': 1,
        'b': 2,
        'q': 3,
        'k': 4,
        'p': 5
    }

    useEffect(() => {
        fetchDataW((data) => {
            // console.log("FIRST TRY BABY!")
            setResponse(JSON.parse(data));
        }, "data");
        fetchDataW((data) => {
            // console.log("FIRST TRY BABY!")
            setEvaluation(JSON.parse(data));
            setGotEvalution(true);
        }, "eval");
    }, []);
    async function fetchDataW(func, dataPoint) {
        try {
            const response = await fetch(
                "http://localhost:8080/" + dataPoint
            );
            const data = await response.text();
            console.log(data);
            console.log("Response from C++ server:", JSON.parse(data));
            func(data);
            setResonded(true);
        } catch (error) {
            console.log("Error fetching data:", error);
            setResponse("Failed to fetch board data, trying again...");
            setTimeout(fetchDataW(func, dataPoint), 1000);
        }
    }

    async function handleSubmit(e) {
        e.preventDefault();
        if(moveInput.length==4){
            
            const controller = new AbortController();
            const signal = controller.signal;
        
            // Set a timeout to abort the request
            const timeoutId = setTimeout(() => controller.abort(), 30000);

            let fileFrom = moveInput[0];
            let rankFrom = moveInput[1];
            let xFrom = fileToX[fileFrom];
            let yFrom = 8 - rankFrom;

            let fileTo = moveInput[2];
            let rankTo = moveInput[3];
            let xTo = fileToX[fileTo];
            let yTo = 8 - rankTo;
            
            let posOfMove = xTo + (yTo * 8); 
            let pieceTypeChar = response[yFrom][xFrom].toLowerCase();
            let pieceTypeNum = pieceToNumber[pieceTypeChar];

            let moveOrCapture = response[yTo][xTo]!=" ";
            console.log("pieceType: ", pieceTypeChar);
            let moveDesc = {
                pieceMovingColor: color,
                pieceType: pieceTypeNum,
                posOfMove:posOfMove,
                moveOrCapture: moveOrCapture,
                xFrom:xFrom,
                yFrom:yFrom
};
            applyMove(xFrom, yFrom, xTo, yTo, pieceTypeChar);
            try {
                const response = await fetch(
                    "http://localhost:8080/MoveResponse",
                    {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                        },
                        body: JSON.stringify({prevMove: moveDesc, timeLeft: 200000}) // Send the move data as JSON
                        // signal:signal
                    }
                );
                clearTimeout(timeoutId);
                const data = await response.text();
                console.log(data);
                console.log("Response from C++ server:", JSON.parse(data));
                setResponse(JSON.parse(data).newPos);
            } catch (error) {
                console.log("Error fetching data:", error);
            }
        }
    }
    function handleInput(e){
        setMoveInput(e.target.value);
    }
    function applyMove(xFrom, yFrom, xTo, yTo, pieceType){
        setResponse(prevBoardState => {
            let res = structuredClone(prevBoardState);
            res[yFrom][xFrom] = " ";
            console.log("res[yFrom][xFrom]: ", res[yFrom][xFrom]);
            res[yTo][xTo] = color ? pieceType.toUpperCase() : pieceType;
            return res;
        })
    }

    //*Stupid, for some reason it goes bottom to top. Going to go backwards to reverse it.
    let squaresEls = [];
    if (responded) {
        for (let i = 0; i < 64; i++) {
            let y = Math.floor(i / 8);
            let x = i % 8;
            // console.log("x:",x,"y:",y)
            let pieceChar = response[y][x];
            console.log(response);
            console.log(responded);
            // console.log((bitboard!=-1 && bitboard[currentBitboard][y][x]));
            let colored = false;
            if (
                posMoves != -1 &&
                currPieceType != -1 &&
                currPiece != null &&
                capOrMove != -1
            ) {
                let currPieceTypeType = posMoves.pieceTypes[currPieceType];
                console.log("Currpiecetype:" + currPieceType);
                if (currPiece == -1) {
                    //Means it is looking at the combined BB
                    colored =
                        capOrMove == 0
                            ? currPieceTypeType.combinedMoveBB[y][x]
                            : currPieceTypeType.combinedCapBB[y][x];
                } else {
                    console.log(
                        "currPieceTypeType:" +
                        JSON.stringify(currPieceTypeType)
                    );
                    console.log(
                        "currPieceTypeType.pieces:" +
                        JSON.stringify(currPieceTypeType.pieces)
                    );
                    console.log("currPiece:" + JSON.stringify(currPiece));
                    console.log(
                        "currPieceTypeType.pieces[currPiece]:" +
                        JSON.stringify(
                            currPieceTypeType.pieces[currPiece]
                        )
                    );
                    if (capOrMove == 0) {
                        colored =
                            currPieceTypeType.pieces[currPiece]
                                .moveBitboard[y][x];
                    } else {
                        colored =
                            currPieceTypeType.pieces[currPiece]
                                .capBitboard[y][x];
                    }
                }
            }

            squaresEls.push(
                <Square
                    tileColor={(x + y) % 2}
                    piece={pieceChar}
                    pieceSvg={
                        reference[
                        pieceChar.toLowerCase() +
                        (pieceChar == pieceChar.toUpperCase()
                            ? "w"
                            : "b")
                        ]
                    }
                    colored={colored}
                    color="#FF0000"
                />
            );
        }
    }
    let pieceTypeButtons = [];
    let piecesButtons = [];

    if (posMoves != -1) {
        for (let i = 0; i < posMoves.pieceTypes.length; i++) {
            let pieceType = posMoves.pieceTypes[i];
            let pieceTypeChar = pieceType.pieceChar;
            if (pieceType.pieces.length == 0) {
                continue;
            }
            pieceTypeButtons.push(
                <>
                    <button
                        className={
                            "pieceButtons" +
                            (currPieceType == i ? " greyed" : "")
                        }
                        onClick={() => {
                            setCurrPieceType(i);
                            setCurrPiece(-1);
                        }}
                    >
                        {pieceCharToFullPieceName[pieceTypeChar]}
                    </button>
                </>
            );
        }
        if (currPieceType != -1) {
            piecesButtons.push(
                <>
                    <button
                        className={
                            "pieceButtons" +
                            (capOrMove == 0 ? " greyed" : "")
                        }
                        onClick={() => {
                            setCapOrMove(0);
                        }}
                    >
                        Move Bitboards
                    </button>
                </>
            );
            piecesButtons.push(
                <>
                    <button
                        className={
                            "pieceButtons" +
                            (capOrMove == 1 ? " greyed" : "")
                        }
                        onClick={() => {
                            setCapOrMove(1);
                        }}
                    >
                        Capture Bitboards
                    </button>
                </>
            );
            piecesButtons.push(
                <>
                    <br />
                </>
            );
            if (capOrMove != -1) {
                if (posMoves.pieceTypes[currPieceType].pieces.length != 0) {
                    piecesButtons.push(
                        <>
                            <button
                                className={
                                    "pieceButtons" +
                                    (currPiece == -1 ? " greyed" : "")
                                }
                                onClick={() => setCurrPiece(-1)}
                            >
                                {capOrMove == 0
                                    ? "Piece Move Combined Bitboard"
                                    : "Piece Capture Combined Bitboard"}
                            </button>
                        </>
                    );
                }

                piecesButtons.push(
                    <>
                        <br />
                    </>
                );

                for (
                    let i = 0;
                    i < posMoves.pieceTypes[currPieceType].pieces.length;
                    i++
                ) {
                    let piece =
                        posMoves.pieceTypes[currPieceType].pieces[i];
                    piecesButtons.push(
                        <>
                            <button
                                className={
                                    "pieceButtons" +
                                    (currPiece == i ? " greyed" : "")
                                }
                                onClick={() => {
                                    setCurrPiece(i);
                                }}
                            >
                                {i + 1}
                            </button>
                        </>
                    );
                }
            }
        }
    }
    let evalBarHeight = 0;
    if (gotEvaluation) {
        let evalBarHeightPercent = 1 / (1 + Math.exp(-0.25 * evaluation));
        evalBarHeight = 400 * evalBarHeightPercent;
    }

    return (
        <div>
            <header>
                <h1>The ACE</h1>
                <p>{response}</p>
            </header>
            <div className="boardAndEval">
                <div className="tiles">{squaresEls}</div>
                <div className="eval--Container">
                    <div
                        className="eval--WhiteBar"
                        style={{ height: evalBarHeight }}
                    ></div>
                </div>
            </div>
            <div className="buttons">
                <div className="buttons--bitboardButtons">
                    <button
                        className="bitboardButton"
                        onClick={() =>
                            fetchDataW((data) => {
                                setPosMoves(JSON.parse(data));
                            }, "getBitboards")
                        }
                    >
                        {posMoves == -1 ? "Get" : "Refresh"} bitboard
                    </button>

                    {posMoves != -1 && (
                        <button
                            className="bitboardButton"
                            onClick={() =>
                                setbitboardVisibility(
                                    (visibility) => !visibility
                                )
                            }
                        >
                            Turn bitboards{" "}
                            {bitboardVisibility ? "Off" : "On"}
                        </button>
                    )}
                    {pieceTypeButtons}

                    <br />

                    {currPieceType != -1 && piecesButtons}
                </div>
                <form className="buttons--moveButtons" onSubmit={handleSubmit}>
                    <button
                        className="enterMoves"
                    >
                        Send move
                    </button>
                    <input 
                        placeholder="Enter move"
                        onInput = {handleInput}
                        value = {moveInput}
                    />
                </form>
            </div>
        </div>
    );
}

export default App;
