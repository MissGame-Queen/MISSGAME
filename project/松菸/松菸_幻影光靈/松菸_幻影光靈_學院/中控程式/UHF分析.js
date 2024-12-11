

let arrData = JSON.parse(variable.get("value"));
let arr_1 = [];
let arr_2 = [];
let arr_3 = [];
let arr_4 = [];
let arr_5 = [];
let arr_6 = [];
let arr_Fruits = [];
const arrWeapon = [
    "EPC_杖",
    "EPC_矛",
    "EPC_槌",
    "EPC_鍊",
    "EPC_劍",
    "EPC_斧",
]
const arrSpiritStone = [
    "EPC_靈石杖",
    "EPC_靈石矛",
    "EPC_靈石槌",
    "EPC_靈石鍊",
    "EPC_靈石劍",
    "EPC_靈石斧",
]
const lantern = "EPC_提燈";
var strTEST = "";

try {
    let str = variableTable.name["魔藥室已登記水果"]?.get("value");
    if (str === "") str = "[]";
    arr_Fruits = JSON.parse(str ?? "[]");

    let testStr = "TEST\n";

    arrData.forEach(item => {
        //[ ]武器登錄系統
        if (item.ant === 1) {

            testStr += "item.ant === 1,";

            arrWeapon.forEach(itemWeapon => {
                let arr = [];
                arr = variableTable.name[itemWeapon]?.get("value") ?? ["變數為空"];
                arr = JSON.parse(variableTable.name[itemWeapon]?.get("value"));
                arr.forEach(itemEPC => {
                    JSON.stringify(itemEPC);
                    if (itemEPC[1] == item.epc) {
                        let jsonData = {
                            "type": itemEPC[0],
                            "value": 0,
                            "spiritstone": []
                        };
                        arr_1.push(jsonData);
                    }
                })
            }
            )
        }
        else if (item.ant === 2 || item.ant === 3 || item.ant === 4) {
            //[ ]水果登錄系統

            if (item.ant === 3) {


                let strFruits = variableTable.name["EPC_魔藥室水果"]?.get("value");
                if (strFruits !== "") {
                    let arr = JSON.parse(strFruits) ?? [];

                    arr.forEach(itemFruitsEPC => {
                        if (itemFruitsEPC === item.epc) {
                            arr_Fruits.push(item.epc);
                        }
                    });
                }

            }

        }
        else if (item.ant === 5) {
            //[ ]屍禁提燈登入系統
            let arr = [];
            arr = variableTable.name[lantern]?.get("value") ?? ["變數為空"];
            arr = JSON.parse(variableTable.name[lantern]?.get("value"));
            arr.forEach(itemEPC => {
                JSON.stringify(itemEPC);
                let strisnull=variableTable.name["GV_觸發提燈變化"]?.get("value")??"";
                
                if (itemEPC[1] == item.epc&&strisnull=="") {
                     let date= new Date();
                     let formattedDate = `${date.getFullYear()}/${
                        String(date.getMonth() + 1).padStart(2, '0')}/${
                        String(date.getDate()).padStart(2, '0')} ${
                        String(date.getHours()).padStart(2, '0')}:${
                        String(date.getMinutes()).padStart(2, '0')}:${
                        String(date.getSeconds()).padStart(2, '0')}`;
                    let jsonData = {
                        "log":formattedDate,
                        "id": itemEPC[0],
                        "value": 4
                    };
                    arr_6.push(jsonData);
                }
            })
        }
    });
} catch (error) {

}


//[v]武器登錄系統
if (arr_1.length > 0) {
    let arr = [];
    try {
        arr = JSON.parse(variableTable.name["斜角巷參數"]?.get("value"))
    } catch (error) {

    }
    arr_1.forEach(item => {
        let has = false;
        arr.forEach(obj => {
            if (obj.type === item.type) {
                has = true;
            }
        })
        if (!has) {
            arr.push(item);
            variableTable.name["武器登錄音效"]?.save("value",
                String(parseInt(variableTable.name["武器登錄音效"]?.get("value")) + 1));
        }
    })



    //[v]COIN系統
    {
        let hasCoin = false;
        arr.forEach(obj => {
            if (obj.type === "累計COIN") {
                hasCoin = true;
            }
        });
        if (!hasCoin) {
            let jsonData = {
                "type": "累計COIN",
                "value": 0
            };
            arr.push(jsonData);
        }
        variableTable.name["斜角巷參數"]?.save("value", JSON.stringify(arr));
    }

    variableTable.name["斜角巷參數"]?.save("value", JSON.stringify(arr));
    let str = variableTable.name["斜角巷參數"]?.get("value");

    let strRoom = "斜角巷";

    arr = JSON.parse(str);

    arr.forEach(item => {

        let json = {};
        let strID = "";
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
            if (item.type?.includes(strType)) {
                strID = item.type;
                strID = strID.substring(strID.length - 2);
                json = {
                    "id": strID,
                    "value": 0,
                };
                (variableTable.name[strRoom + strType]?.save("value", JSON.stringify(json)));
            }

        }

    });
}

//[v]靈石登錄系統
for (let i = 1; i < 5; i++) {
    let arrI = [];
    let configName = "";

    switch (i) {
        case 1:
            arrI = arr_2;
            configName = "校史室參數";
            break;
        case 2:
            arrI = arr_3;
            configName = "魔藥室參數";
            break;
        case 3:
            arrI = arr_4;
            configName = "道具室參數";
            break;
        case 4:
            arrI = arr_5;
            configName = "龍窟參數";
            break;
    }

    if (arrI.length > 0) {

        let arr = [];
        arr = JSON.parse(variableTable.name[configName]?.get("value"));

        arr.forEach(item => {
            arrI.forEach(itemEPC => {
                let str1 = "";
                let str2 = "";
                str1 = JSON.stringify(item);
                str2 = JSON.stringify(itemEPC);
                if (str1.includes("劍") && str2.includes("劍") ||
                    str1.includes("杖") && str2.includes("杖") ||
                    str1.includes("矛") && str2.includes("矛") ||
                    str1.includes("斧") && str2.includes("斧") ||
                    str1.includes("槌") && str2.includes("槌") ||
                    str1.includes("鍊") && str2.includes("鍊")
                ) {
                    if (item.spiritstone.indexOf(itemEPC.epc) == -1)
                        item.spiritstone.push(itemEPC.epc);
                }
            })
        })
        variableTable.name[configName]?.save("value", JSON.stringify(arr));
    }

}

//[v]水果登錄系統
if (arr_Fruits.length > 0) {


    let str = variableTable.name["魔藥室已登記水果"]?.get("value") ?? "";
    let arr = [];
    if (str === "") str = "[]";
    arr = JSON.parse(str);

    arr.forEach(item => {
        arr_Fruits.push(item);
    })
    arr_Fruits = arr_Fruits.filter((item, index) => {
        // 檢查當前元素在陣列中的第一次出現的位置是否等於當前索引
        return arr_Fruits.indexOf(item) === index;
    });
    if (JSON.stringify(arr_Fruits) !== str) {
        variableTable.name["魔藥室已登記水果"]?.save("value", JSON.stringify(arr_Fruits));
        variableTable.name["魔藥室水果"]?.save("value", String(arr_Fruits.length));
    }
}
//[v]提燈登錄系統
if (arr_6.length > 0) {
    variableTable.name["GV_觸發提燈變化"]?.save("value", JSON.stringify(arr_6[0]));
}



