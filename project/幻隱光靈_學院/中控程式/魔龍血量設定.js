


let str = "[]";
try {
    let jsonReturn = [];
    let intOut = 0;
    str = variableTable.name["道具室參數"]?.get("value") ?? "[]";
    if (str !== "") {
        jsonReturn = JSON.parse(str);

        //武器數*2+總武器等級
        jsonReturn.forEach(item => {
            if (item.type.includes("劍") || item.type.includes("杖") || item.type.includes("矛") ||
                item.type.includes("鍊") || item.type.includes("槌") || item.type.includes("斧"))
                intOut++;
        })
    }

    if (intOut === 0 || str === "") intOut = 6;
    let json = {
        "newHealth": 30 + intOut * 5
        //"newHealth": 60
    }

    variableTable.name["GV_設定魔龍血量用"]?.save("value", JSON.stringify(json));

} catch (error) {

}

//variableTable.name["GV_TEST"]?.save("value", "str");
//return;