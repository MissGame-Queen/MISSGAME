/**
 * CSV格式為A1表單名稱
 * B1.2.3...為欄位名稱
 * 之後為每列數據的匯入
 */


const Parse = require('parse/node');

/**
 * 初始化 Parse Server
 */
Parse.initialize(
    "NumLockEscapeServerIsASystemForEscapeRoomDesignID",
    "",
    "NumLockEscapeServerIsASystemForEscapeRoomDesignMaster"
);
Parse.serverURL = "http://192.168.0.8:3000/parse";

const fs = require('fs');

// 读取 CSV 文件内容
const csvData = fs.readFileSync('./匯入用/人區音效列表.csv', 'utf8');

// 将 CSV 数据按行分割成数组
//取代\r\n成\n
//刪除多餘空白
//從頭直到搜尋特定字串後push到新陣列並往復此動作
const rows = csvData.replace(/\r\n/g, '\n').trim().split('\n');

// 将每一行再按逗号分割成数组，形成二维数组
//依函數規則複製陣列
var arrImport = rows.map(row => row.split(','));
const eCSV = {
    'name': 0,
    'link': 1,
};
console.log(arrImport[0][0]);
for (let i = 0; i < arrImport[1].length; i++)
    console.log(arrImport[1][i]);
return;
//console.log(arrImport);
var csvString = '\uFEFF';
/**
 * 匹配csv中的link並套用對應
 */
//new Parse.Query(arrImport[0][0]).addAscending
new Parse.Query(arrImport[0][0]).limit(200).find().then(list => {
    // return;
    //arrImport.push(['name', 'link', 'IODevice']);
    list.reverse().forEach(item => {
        let link = item.get("link");
        let name = item.get("name");
        let arr = [];
        if (link !== name) {
            arr.push(name);
            arr.push(link);
            arrImport.push(arr);
        }
    });
    csvString += (arrImport.map(row => row.join(',')).join('\n')) + '\n';
    //console.log(csvString);

})
//?記得加'\uFEFF'不然Excel打開會是亂碼
//fs.writeFileSync('IO紀錄表.csv', csvString, 'utf8');