import React, { useState } from 'react';


function Square({tileColor, piece,pieceSvg}) {
    return ( 
        <div className='tile' style={{"backgroundColor":tileColor==1?"White":"Grey","color":tileColor==0?"White":"Black"}}>
            <img 
            // src={'./assets/'+(piece.toLowerCase()) + ((piece==piece.toUpperCase())?"b":"w")+".svg"}
            src={pieceSvg}
            />
        </div>
     );
}

export default Square;