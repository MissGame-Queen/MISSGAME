let initI = 0;
let str = ""
let json = {};
let arr = [];
let strRoom = "";
let roomCoin = 0;
let allCoin = 0;
let strTEST = "";
try {
    for (let i = initI; i <= (initI == 0 ? 5 : initI); i++) {
        switch (i) {
            case 0:
                strRoom = "斜角巷";
                break;
            case 1:
                strRoom = "校史室";
                break;
            case 2:
                strRoom = "魔藥室";
                break;
            case 3:
                strRoom = "道具室";
                break;
            case 4:
                strRoom = "龍窟";
                break;
        }
        str = variableTable.name[strRoom + "參數"]?.get("value");

        if (str !== "") {
            arr = JSON.parse(str);
            let ifnotroomcoin = true;
            arr.forEach(item => {
                if (item.type === "累計COIN") {
                    item.value = parseInt(item.value) + 1;
                }
                if (item.type === strRoom + "COIN") {                    
                    ifnotroomcoin = false;
                    item.value = parseInt(item.value) + 1;
                    roomCoin = item.value;
                }
            });
            if (ifnotroomcoin) {
                let strRoomCoin = strRoom + "COIN";
                roomCoin = 1;
                let json = {
                    "type": strRoomCoin,
                    "value": roomCoin
                }
                arr.push(json);
            }
        }        
        variableTable.name[strRoom + "參數"]?.save("value", JSON.stringify(arr));
        variableTable.name[strRoom + "COIN"]?.save("value", String(roomCoin));
    }
} catch (error) {

}
//variableTable.name["GV_TEST"]?.save("value", strTEST);