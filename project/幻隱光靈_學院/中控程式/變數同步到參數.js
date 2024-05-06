let str;
let arr;
let strRoom = "";
let strTEST = "";

try {
    //5房間參數
    let allCoin = 0;
    for (let i = 0; i < 5; i++) {
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
            //[v]累計COIN
            arr = JSON.parse(str);
            let strData = variableTable.name[strRoom + "已過關謎題"]?.get("value") ?? "";
            //strTEST+=strData+"\n";
            if (strData != "") {
                let strTypeLevel = JSON.parse(strData) ?? [];
                allCoin += strTypeLevel.length;
                let ifHave = false;

                arr.forEach(item => {
                    if (item.type.includes(strRoom + "已過關謎題")) {
                        item.value = strTypeLevel;
                        ifHave = true;

                    }
                })

                if (ifHave === false) {
                    let json = {
                        "type": strRoom + "已過關謎題",
                        "value": strTypeLevel
                    }
                    arr.push(json);
                }
                //strTEST += JSON.stringify(arr);
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
                            case 6:
                                strType = "COIN";
                                break;
                        }
                        if (i < 6) {
                            let strTypeLevel = variableTable.name[strRoom + strType]?.get("value") ?? "";
                            if (item.type.includes(strType) && strTypeLevel !== "") {
                                let json = JSON.parse(strTypeLevel);
                                item.value = json.value;
                            }
                        }
                        else if (i === 6) {
                            if (item.type.includes(strType)) {
                                item.value = allCoin;
                            }
                        }
                    }

                });
                //strTEST += JSON.stringify(arr);
                variableTable.name[strRoom + "參數"]?.save("value", JSON.stringify(arr));
            }
        }
    }
    //魔龍參數
    str = variableTable.name["GV_魔龍參數參數"]?.get("value") ?? "{}";
    let jsonReturn = JSON.parse(str);
    const keys = Object.keys(jsonReturn);
    keys.forEach(key => {
        if (key === "魔龍累計擊中")
            jsonReturn[key] = variableTable.name["GV_魔龍累計擊中"].get("value");
        else if (key === "魔龍累計閃躲")
            jsonReturn[key] = variableTable.name["GV_魔龍累計閃躲"].get("value");
        else if (key === "魔龍累計星星")
            jsonReturn[key] = variableTable.name["GV_魔龍累計星星"].get("value");
        else if (key === "魔龍累計BOUNS")
            jsonReturn[key] = variableTable.name["GV_魔龍累計BOUNS"].get("value");
        else if (key === "魔龍累計秒數")
            jsonReturn[key] = variableTable.name["GV_魔龍累計秒數"].get("value");
        else if (key === "魔龍目前血量")
            jsonReturn[key] = variableTable.name["GV_魔龍目前血量"].get("value");
    });

    let Time = parseInt(variableTable.name["GV_魔龍累計秒數"]?.get("value"));
    if (Time > 0)
        jsonReturn["魔龍累計秒數"] = Time;
    //variableTable.name["GV_魔龍參數"]?.save("value", JSON.stringify(jsonReturn));  

} catch (e) {
}


//variableTable.name["GV_TEST"]?.save("value", strTEST);