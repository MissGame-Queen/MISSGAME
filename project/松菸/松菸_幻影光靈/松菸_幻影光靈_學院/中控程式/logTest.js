

let arrData = variable.get("value");

    let date= new Date();
    let formattedDate = `${date.getFullYear()}/${
       String(date.getMonth() + 1).padStart(2, '0')}/${
       String(date.getDate()).padStart(2, '0')} ${
       String(date.getHours()).padStart(2, '0')}:${
       String(date.getMinutes()).padStart(2, '0')}:${
       String(date.getSeconds()).padStart(2, '0')}`;
   let jsonData = {
       "log":formattedDate,
       "id": 100,
       "value": 1<<(arrData%3)//二進制
   };
    variableTable.name["GV_觸發提燈變化"]?.save("value", JSON.stringify(jsonData));



//variableTable.name["GV_TEST"]?.save("value", arrData);