


let str = "{}";
let jsonReturn = {};
str = variableTable.name["龍窟參數"]?.get("value") ?? "{}";
jsonReturn = JSON.parse(str);
let intOut = 0;
//武器數*2+總武器等級
jsonReturn.forEach(item => {
    if (item.type.includes("劍") || item.type.includes("杖") || item.type.includes("矛") ||
        item.type.includes("鍊") || item.type.includes("槌") || item.type.includes("斧"))
        intOut += parseInt(item.value) + 2;
})
let json = {
    "value": intOut
}

variableTable.name["GV_龍窟複數出球"]?.save("value", JSON.stringify(json));

//variableTable.name["GV_TEST"]?.save("value", JSON.stringify(json));
//return;