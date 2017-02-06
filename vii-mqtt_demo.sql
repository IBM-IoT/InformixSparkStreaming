database stores_demo;

-- Add a couple of call type records to allow for this demo to be more realistic
insert into call_type (call_code,code_descr) values ("P","place order");
insert into call_type (call_code,code_descr) values ("U","update order");
insert into call_type (call_code,code_descr) values ("C","modify contact info");

-- Add a new customer record
insert into customer ( customer_num, fname, lname, company, address1, address2, city, state, zipcode, phone )
       values ( 0, "Salesman", "Joe", "Peanuts Memorabilia", "1770 James Street", NULL, "Hennepin County",
                "MN", "55304", "612-348-3000" );

-- Add a customer call record
insert into cust_calls ( customer_num, call_dtime, user_id, call_code, call_descr, res_dtime, res_descr )
       values ( dbinfo('sqlca.sqlerrd1'), "2017-01-05 09:13", "charlie", "P",
                "Initial order by customer. Customer is looking for a new suplier to meet their needs.",
                NULL, NULL );

-- Add the order record
insert into orders ( order_num, order_date, customer_num, ship_instruct, backlog, po_num, ship_date, ship_weight,
                     ship_charge, paid_date )
       values ( 0, "01/05/2017", (select customer_num from customer where company="Peanuts Memorabilia"),
                "Avoid Lucy", "n", "CMZ81452", "01/07/2017", 493.61, 253.39, NULL );

-- Add the item record
insert into items ( item_num, order_num, stock_num, manu_code, quantity, total_price )
       values ( 1, dbinfo('sqlca.sqlerrd1'), 4, "HRO", 10, 4800.00 );

-- Update the initial customer call record
update cust_calls set
   res_dtime="2017-01-05 09:49",
   res_descr="Initial customer setup. Order 10 cases of footballs. If we do well we will get a contract with customer."
where customer_num=(select customer_num from customer where company="Peanuts Memorabilia") and res_dtime is NULL;

-- The customer is now setup and their first order placed. Now let do some modifications to the order
-- The customer calls back to correct their shipping information
-- Add a customer call record
insert into cust_calls ( customer_num, call_dtime, user_id, call_code, call_descr, res_dtime, res_descr )
       values ( (select customer_num from customer where company="Peanuts Memorabilia"),
                "2017-01-05 09:54", "charlie", "C",
                "Customer called back and stated they gave us the wrong zip code",
                NULL, NULL );

-- Make the update to the customer table
update customer set zipcode='55304' where company="Peanuts Memorabilia";

-- Update the customer call record
update cust_calls set
   res_dtime="2017-01-05 09:57",
   res_descr="Updated customer info and thanked them."
where customer_num=(select customer_num from customer where company="Peanuts Memorabilia") and res_dtime is NULL;

-- The custmoer calls back and wants to add to their order
-- Add a customer call record
insert into cust_calls ( customer_num, call_dtime, user_id, call_code, call_descr, res_dtime, res_descr )
       values ( (select customer_num from customer where company="Peanuts Memorabilia"),
                "2017-01-05 13:17", "charlie", "U",
                "Customer wanted to modify their order.",
                NULL, NULL );

-- Add an item record
-- This needs to be in a transaction;
begin work;
insert into items ( item_num, order_num, stock_num, manu_code, quantity, total_price )
       values ( 2, (select order_num from orders where po_num="CMZ81452"), 3, "HSK", 4, 960.00 );

-- Update the orders table
update orders set
   ship_weight=974.78, ship_charge=816.35
where
       po_num="CMZ81452"
   and customer_num=(select customer_num from customer where company="Peanuts Memorabilia");
commit work;

-- OOPS!!! The salesperson realized they added the wrong item.
-- Delete the item record.
-- This needs to be in a transaction;
begin work;
delete from items where
   order_num=(select order_num from orders where customer_num=
      (select customer_num from customer where company="Peanuts Memorabilia"))
   and stock_num=3 and manu_code="HSK";

-- Update the orders table
update orders set
   ship_weight=493.61, ship_charge=253.39
where
       po_num="CMZ81452"
   and customer_num=(select customer_num from customer where company="Peanuts Memorabilia");
commit work;

-- Add the right item record
-- This needs to be in a transaction;
begin work;
insert into items ( item_num, order_num, stock_num, manu_code, quantity, total_price )
       values ( 2, (select order_num from orders where po_num="CMZ81452"), 2, "HRO", 4, 504.00 );

-- Update the orders table
update orders set
   ship_weight=516.12, ship_charge=293.78
where
       po_num="CMZ81452"
   and customer_num=(select customer_num from customer where company="Peanuts Memorabilia");
commit work;

-- Update the customer call table table
update cust_calls set
   res_dtime="2017-01-05 13:28",
   res_descr="Added 4 cases of baseballs to order."
where
   customer_num=(select customer_num from customer where company="Peanuts Memorabilia")
   and res_dtime is NULL;

-- Don't forget we get paid or we don't stay in business
-- When we do we must update the orders table
update orders
   set paid_date="01/06/2017"
where
   customer_num=(select customer_num from customer where company="Peanuts Memorabilia")
   and paid_date is NULL;

-- Now let's see what happens when a transaction is rolled back
-- Let's pretend the customer called back
-- Add a customer call record
insert into cust_calls ( customer_num, call_dtime, user_id, call_code, call_descr, res_dtime, res_descr )
       values ( (select customer_num from customer where company="Peanuts Memorabilia"),
                "2017-01-10 11:32", "charlie", "U",
                "Customer expressing their pleasure using our company.",
                NULL, NULL );

-- and while on the call the customer rep accidently clicked on the call completed button
-- But luckily the customer rep had an "Are you sure?" pop-up
-- This needs to be in a transaction;
begin work;
-- Update the customer call table table
update cust_calls set
   res_dtime="2017-01-10 11:45",
   res_descr="We now have a new permanent customer."
where
   customer_num=(select customer_num from customer where company="Peanuts Memorabilia")
   and res_dtime is NULL;
-- customer rep selects the "No" button
rollback work;

-- Notice nothing was sent to the MQTT broker
-- Now the call actually ends
-- This needs to be in a transaction;
begin work;
-- Update the customer call table table
update cust_calls set
   res_dtime="2017-01-10 11:47",
   res_descr="We now have a new permanent customer."
where
   customer_num=(select customer_num from customer where company="Peanuts Memorabilia")
   and res_dtime is NULL;
-- customer rep selects the "Yes" button
commit work;

-- And the transaction is sent to the MQTT broker

