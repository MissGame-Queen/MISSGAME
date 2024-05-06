let str = variableTable.name["斜角巷參數"]?.get("value") ?? "";
if (str === "") str = "[]";
let arr = JSON.parse(str);
const titel = "手動登錄_";

let strTEST = "";
for (let i = 0; i < 6; i++) {
    let strType = "";
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
    str = variableTable.name[titel + strType]?.get("value") ?? "";
    if (str !== "") {
        strTEST += str + "\n";
        let isHave = false;
        arr.forEach(item => {
            if (item.type.includes(strType)) { 
                item.type = strType + str;
                item["value"] = 0;
                item["spiritstone"] = [];
                let json={
                    "id":parseInt(str),
                    "value":0
                }
                variableTable.name["斜角巷" + strType]?.save("value", JSON.stringify(json));
                isHave=true;
            }
        })
        if (isHave === false) {
            let json = {
                "type": strType + str,
                "value": 0,
                "spiritstone": []
            }
            arr.push(json);
            json={
                "id":parseInt(str),
                "value":0
            }
            variableTable.name["斜角巷" + strType]?.save("value", JSON.stringify(json));
        }
    }

}
variableTable.name["斜角巷參數"]?.save("value", JSON.stringify(arr));
//variableTable.name["GV_TEST"]?.save("value", JSON.stringify(arr));