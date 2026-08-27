

-- ------------------------------------------------------------
-- 1. CUSTOMER
-- ------------------------------------------------------------
CREATE TABLE Customer (
    customer_id     NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL,
    email           VARCHAR2(100)   NOT NULL UNIQUE,
    phone           VARCHAR2(15)    NOT NULL UNIQUE,
    password        VARCHAR2(255)   NOT NULL
);

-- ------------------------------------------------------------
-- 2. RESTAURANT
-- ------------------------------------------------------------
CREATE TABLE Restaurant (
    restaurant_id   NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL,
    email           VARCHAR2(100)   UNIQUE,
    phone           VARCHAR2(15)    UNIQUE,
    balance         NUMBER(10,2)    DEFAULT 0
);

-- ------------------------------------------------------------
-- 3. CATEGORY
-- ------------------------------------------------------------
CREATE TABLE Category (
    category_id     NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL
);

-- ------------------------------------------------------------
-- 4. RIDER
-- ------------------------------------------------------------
CREATE TABLE Rider (
    rider_id        NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL,
    phone           VARCHAR2(15)    NOT NULL UNIQUE,
    vehicle         VARCHAR2(50),
    license_number  VARCHAR2(30)    UNIQUE,
    experience      NUMBER(4,1),
    rating          NUMBER(2,1),
    balance         NUMBER(10,2)    DEFAULT 0
);

-- ------------------------------------------------------------
-- 5. FOODITEM  (belongs to Restaurant, belongs to Category)
-- ------------------------------------------------------------
CREATE TABLE FoodItem (
    item_id         NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL,
    description     VARCHAR2(500),
    image           VARCHAR2(255),
    is_available    NUMBER(1)       DEFAULT 1 CHECK (is_available IN (0,1)),
    restaurant_id   NUMBER(10)      NOT NULL,
    category_id     NUMBER(10),
    CONSTRAINT fk_fooditem_restaurant FOREIGN KEY (restaurant_id)
        REFERENCES Restaurant(restaurant_id) ON DELETE CASCADE,
    CONSTRAINT fk_fooditem_category FOREIGN KEY (category_id)
        REFERENCES Category(category_id) ON DELETE SET NULL
);

-- ------------------------------------------------------------
-- 6. ADD-ON  (belongs to a FoodItem)
-- ------------------------------------------------------------
CREATE TABLE Addon (
    addon_id        NUMBER(10)      PRIMARY KEY,
    name            VARCHAR2(100)   NOT NULL,
    price           NUMBER(8,2)     NOT NULL,
    item_id         NUMBER(10)      NOT NULL,
    CONSTRAINT fk_addon_fooditem FOREIGN KEY (item_id)
        REFERENCES FoodItem(item_id) ON DELETE CASCADE
);

-- ------------------------------------------------------------
-- 7. ORDER  (placed by Customer, received by Restaurant)
-- ------------------------------------------------------------
CREATE TABLE "ORDER" (
    order_id            NUMBER(10)      PRIMARY KEY,
    customer_id         NUMBER(10)      NOT NULL,
    restaurant_id       NUMBER(10)      NOT NULL,
    delivery_address    VARCHAR2(255)   NOT NULL,
    delivery_fee        NUMBER(8,2)     DEFAULT 0,
    discount            NUMBER(8,2)     DEFAULT 0,
    total_amount        NUMBER(10,2)    NOT NULL,
    order_time          TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_order_customer FOREIGN KEY (customer_id)
        REFERENCES Customer(customer_id),
    CONSTRAINT fk_order_restaurant FOREIGN KEY (restaurant_id)
        REFERENCES Restaurant(restaurant_id)
);

-- ------------------------------------------------------------
-- 8. ORDER ITEM  (contains FoodItem, included in Order)
-- ------------------------------------------------------------
CREATE TABLE Order_Item (
    order_item_id   NUMBER(10)      PRIMARY KEY,
    order_id        NUMBER(10)      NOT NULL,
    item_id         NUMBER(10)      NOT NULL,
    quantity        NUMBER(5)       NOT NULL CHECK (quantity > 0),
    price           NUMBER(8,2)     NOT NULL,
    CONSTRAINT fk_orderitem_order FOREIGN KEY (order_id)
        REFERENCES "ORDER"(order_id) ON DELETE CASCADE,
    CONSTRAINT fk_orderitem_fooditem FOREIGN KEY (item_id)
        REFERENCES FoodItem(item_id)
);

-- ------------------------------------------------------------
-- 9. WISHLIST  (belongs to a Customer)
-- ------------------------------------------------------------
CREATE TABLE Wishlist (
    wishlist_id     NUMBER(10)      PRIMARY KEY,
    customer_id     NUMBER(10)      NOT NULL,
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_wishlist_customer FOREIGN KEY (customer_id)
        REFERENCES Customer(customer_id) ON DELETE CASCADE
);

-- ------------------------------------------------------------
-- 10. WISHLIST ITEM  (contains FoodItem, included in Wishlist)
-- ------------------------------------------------------------
CREATE TABLE Wishlist_Item (
    wishlist_item_id    NUMBER(10)  PRIMARY KEY,
    wishlist_id         NUMBER(10)  NOT NULL,
    item_id             NUMBER(10)  NOT NULL,
    CONSTRAINT fk_wishlistitem_wishlist FOREIGN KEY (wishlist_id)
        REFERENCES Wishlist(wishlist_id) ON DELETE CASCADE,
    CONSTRAINT fk_wishlistitem_fooditem FOREIGN KEY (item_id)
        REFERENCES FoodItem(item_id)
);

-- ------------------------------------------------------------
-- 11. REVIEW  (written by Customer, rated on Restaurant)
-- ------------------------------------------------------------
CREATE TABLE Review (
    review_id       NUMBER(10)      PRIMARY KEY,
    customer_id     NUMBER(10)      NOT NULL,
    restaurant_id   NUMBER(10)      NOT NULL,
    rating          NUMBER(2,1)     CHECK (rating BETWEEN 0 AND 5),
    comment         VARCHAR2(500),
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_review_customer FOREIGN KEY (customer_id)
        REFERENCES Customer(customer_id) ON DELETE CASCADE,
    CONSTRAINT fk_review_restaurant FOREIGN KEY (restaurant_id)
        REFERENCES Restaurant(restaurant_id) ON DELETE CASCADE
);

-- ------------------------------------------------------------
-- 12. SUPPORT TICKET  (raised by Customer)
-- ------------------------------------------------------------
CREATE TABLE Support_Ticket (
    ticket_id       NUMBER(10)      PRIMARY KEY,
    customer_id     NUMBER(10)      NOT NULL,
    subject         VARCHAR2(150)   NOT NULL,
    description     VARCHAR2(1000),
    status          VARCHAR2(20)    DEFAULT 'OPEN',
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_ticket_customer FOREIGN KEY (customer_id)
        REFERENCES Customer(customer_id) ON DELETE CASCADE
);

-- ------------------------------------------------------------
-- 13. DELIVERY  (refers to Order, delivered by Rider)
-- ------------------------------------------------------------
CREATE TABLE Delivery (
    delivery_id     NUMBER(10)      PRIMARY KEY,
    order_id        NUMBER(10)      NOT NULL UNIQUE,
    rider_id        NUMBER(10),
    pickup_time     TIMESTAMP,
    delivery_time   TIMESTAMP,
    status          VARCHAR2(20)    DEFAULT 'PENDING',
    CONSTRAINT fk_delivery_order FOREIGN KEY (order_id)
        REFERENCES "ORDER"(order_id) ON DELETE CASCADE,
    CONSTRAINT fk_delivery_rider FOREIGN KEY (rider_id)
        REFERENCES Rider(rider_id)
);

-- ------------------------------------------------------------
-- 14. PAYMENT  (paid for an Order)
-- ------------------------------------------------------------
CREATE TABLE Payment (
    payment_id      NUMBER(10)      PRIMARY KEY,
    order_id        NUMBER(10)      NOT NULL UNIQUE,
    method          VARCHAR2(30)    NOT NULL,
    amount          NUMBER(10,2)    NOT NULL,
    transaction_id  VARCHAR2(50),
    status          VARCHAR2(20)    DEFAULT 'PENDING',
    CONSTRAINT fk_payment_order FOREIGN KEY (order_id)
        REFERENCES "ORDER"(order_id) ON DELETE CASCADE
);

