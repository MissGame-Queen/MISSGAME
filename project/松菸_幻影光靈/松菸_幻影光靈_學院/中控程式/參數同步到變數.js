
let str = ""
let json = {};
let arr = [];
let strRoom = "";


try {
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
        if (str === ""||str === "[]") {
            (variableTable.name[strRoom + "劍"]?.save("value", ""));
            (variableTable.name[strRoom + "杖"]?.save("value", ""));
            (variableTable.name[strRoom + "矛"]?.save("value", ""));
            (variableTable.name[strRoom + "斧"]?.save("value", ""));
            (variableTable.name[strRoom + "槌"]?.save("value", ""));
            (variableTable.name[strRoom + "鍊"]?.save("value", ""));
        }
        else if (i > 0) {
            arr = JSON.parse(str);

            arr.forEach(item => {
                let json = {};
                let strID;
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
                    if (i < 6) {
                        if (item.type?.includes(strType)) {
                            strID = item.type;
                            strID = strID.substring(strID.length - 2);
                            json = {
                                "id": parseInt(strID),
                                "value": item.value,
                            };
                            (variableTable.name[strRoom + strType]?.save("value", JSON.stringify(json)));
                        }
                    }
                }
            });
        }
    }

    str = variableTable.name["GV_魔龍參數"]?.get("value");
    if (str === "" || str === "{}") {
        (variableTable.name["GV_魔龍累計秒數"]?.save("value", ""));
        (variableTable.name["GV_魔龍目前血量"]?.save("value", ""));
        (variableTable.name["GV_魔龍累計閃躲"]?.save("value", ""));
        (variableTable.name["GV_魔龍累計擊中"]?.save("value", ""));
        (variableTable.name["GV_魔龍累計星星"]?.save("value", ""));
        (variableTable.name["GV_魔龍累計BOUNS"]?.save("value", ""));
    }
} catch (error) {

}

