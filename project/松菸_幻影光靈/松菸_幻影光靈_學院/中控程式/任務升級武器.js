
let initI = 0;

let str = ""
let json = {};
let arr = [];
let strRoom = "";
let strTEST = "";
try {
    initI = option.i ?? 0;
    for (let i = initI; i <= (initI == 0 ? 6 : initI); i++) {
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
            case 5:
                strRoom = "龍窟";
                break;
        }
        str = variableTable.name[strRoom + "參數"]?.get("value");
        if (str !== "") {
            if (i < 4) {
                let strmission = variableTable.name[strRoom + "已過關謎題"]?.get("value") ?? "";
                if (strmission !== "") {
                    let arrFinish = JSON.parse(strmission);
                    //let isFinish=false;
                    arr = JSON.parse(str);
                    arr.forEach(item => {
                        arrFinish.forEach(itemFinish => {
                            let strType = "";
                            for (let j = 0; j < 7; j++) {
                                switch (j) {
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
                                    let strID = item.type;
                                    strID = strID.substring(strID.length - 2);
                                    json = {
                                        "id": parseInt(strID),
                                        "value": item.value,
                                    };
                                    (variableTable.name[strRoom + strType]?.save("value", JSON.stringify(json)));
                                    variableTable.name[`GV_觸發${strType}燈光變化`].save("value", JSON.stringify(json));
                                }
                            }
                        });
                    })
                }
                variableTable.name[strRoom + "參數"]?.save("value", JSON.stringify(arr));
            }
            //如果是龍窟燈光秀
            else if (i >= 4) {
                variableTable.name["GV_TEST"]?.save("value",str);
                arr = JSON.parse(str);
                let strType = "";
                arr.forEach(item => {
                    for (let j = 0; j < 6; j++) {
                        switch (j) {
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
                            if (i === 4)
                                item.value = 3;
                            else if (i === 5)
                                item.value = 0;
                            let strID = item.type;
                            strID = strID.substring(strID.length - 2);
                            json = {
                                "id": parseInt(strID),
                                "value": item.value,
                            };
                            (variableTable.name[strRoom + strType]?.save("value", JSON.stringify(json)));
                            variableTable.name[`GV_觸發${strType}燈光變化`].save("value", JSON.stringify(json));
                        }
                    }
                });
            }
        }
    }

} catch (error) {

}


//variableTable.name["GV_TEST"]?.save("value",strTEST);
