create index customer_viimqtt on customer(fname,lname,company,address1,address2,city,state,zipcode,phone)
        using informix_socket_streaming(host='localhost',port='1883',topic='customer');
create index cust_calls_viimqtt on cust_calls(customer_num,call_dtime,call_code,call_descr,res_dtime,res_descr)
        using informix_socket_streaming(host='localhost',port='1883',topic='customer');
create index orders_viimqtt on orders(order_num,order_date,customer_num,backlog,po_num,ship_date,ship_weight,
        ship_charge,paid_date) using informix_socket_streaming(host='localhost',port='1883',topic='orders');
create index items_viimqtt on items(item_num,stock_num,manu_code,quantity,total_price) using
        informix_socket_streaming(host='localhost',port='1883',topic='orders');
