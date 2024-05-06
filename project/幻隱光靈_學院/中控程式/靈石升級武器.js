let initI = 0;
let str = ""
let json = {};
let arr = [];
let strRoom = "";
let strTEST = "";
try {
    initI = option.i ?? 0;
    for (let i = initI; i <=(initI == 0 ? 5 : initI); i++) {
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
            arr.forEach(item => {
                let strType = "";
                for (let i = 0; i < 6; i++) {
                    switch (i) {
                        case 0:
                            strType = "劍";
                            break;
                        case 1:
                            strType = "杖";
                            break;
                        case 2:
                            strType = "矛";
                            break;
                        case 3:
                            strType = "斧";
                            break;
                        case 4:
                            strType = "槌";
                            break;
                        case 5:
                            strType = "鍊";
                            break;
                    }

                    if (item.type.includes(strType)) {
                        let json = JSON.parse(variableTable.name[strRoom + strType]?.get("value"));
                        json.value = item.spiritstone.length;
                        (variableTable.name[strRoom + strType]?.save("value", JSON.stringify(json)));
                    }
                }
            });

        }
    }
} catch (error) {

}
//variableTable.name["GV_TEST"]?.save("value", strTEST);
