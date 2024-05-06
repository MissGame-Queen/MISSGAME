
let initI = 0;

let str = ""
let json = {};
let arr = [];
let strRoom = "";
let strTEST = "";
try {
    initI = option.i ?? 0;
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
            let strmission = variableTable.name[strRoom + "已過關謎題"]?.get("value") ?? "";
            if (strmission !== "") {
                let arrFinish = JSON.parse(strmission);
                //let isFinish=false;
                arr = JSON.parse(str);
                arr.forEach(item => {
                arrFinish.forEach(itemFinish => {
                        let strType = "";
                        for (let i = 0; i < 7; i++) {
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
                                case 6:
                                    strType = "已過關謎題";
                                    break;
                            }
                            //如果已登記該武器且也有通過相關支線
                            if (item.type.includes(strType) && itemFinish.includes(strType)) {
                                //strTEST += `${item.type},${itemFinish},${item.type.includes(strType)},${itemFinish.includes(strType)}\n`;
                                //武器等級+1                                
                                item.value = parseInt(item.value) + 1;
                            }
                            
                        }

                    });
                })
            }
            variableTable.name[strRoom + "參數"]?.save("value", JSON.stringify(arr));
        }
    }

} catch (error) {

}


//variableTable.name["GV_TEST"]?.save("value",strTEST);
