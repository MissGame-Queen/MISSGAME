
let str = variableTable.name["龍窟參數"]?.get("value");
let arr = [];
if (str !== "") {
    arr = JSON.parse(str);
    let strType = "";
    arr.forEach(item => {
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
                item.value = 0;
            }
        }
    });
    variableTable.name["龍窟參數"]?.save("value", JSON.stringify(arr));
}
//variableTable.name["GV_TEST"]?.save("value",str);