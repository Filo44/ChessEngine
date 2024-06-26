import React, { useState } from 'react';
import cross from "./assets/cross.svg"

function Square({tileColor, piece,pieceSvg,colored,color,num}) {
    return ( 
        <div className='tile' style={{"backgroundColor":tileColor==0?"White":"Grey","color":tileColor==0?"White":"Black"}}>
            <img 
                // src={'./assets/'+(piece.toLowerCase()) + ((piece==piece.toUpperCase())?"b":"w")+".svg"}
                src={pieceSvg}
            />
            {colored && <>
                <img className='Cross' src={cross} style={{color:color}}></img>
            </>}
            {num && <div>
                {num}
            </div>}
        </div>
     );
}

export default Square;