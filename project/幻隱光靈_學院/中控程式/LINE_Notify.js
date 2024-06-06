
let message = "TEST"; // 使用硬编码的消息进行演示
if (!message) {
    console.error("消息内容为空，无法发送请求");
    return;
}
console.log("[LINE]", message);

const url = 'https://notify-api.line.me/api/notify';
const token = "fnBwOwWj1fGtJNq8AzUxYGH0rOFI6gxCNynpjcTV2v6";

fetch(url, {
    method: 'POST',
    headers: {
        'Authorization': `Bearer ${token}`,
        'Content-Type': 'application/x-www-form-urlencoded'
    },
    body: `message=${encodeURIComponent(message)}`
})
.then(response => {
    if (!response.ok) {
        //throw new Error('Network response was not ok');
variableTable.name["GV_TEST"]?.save("value", 'Network response was not ok');

    }
    return response.json();
})
/*
.then(data => {
    console.log("响应数据:", data);
})
.catch(error => {
    console.error("错误:", error);
});
*/
//variableTable.name["GV_TEST"]?.save("value", "LINE2");
