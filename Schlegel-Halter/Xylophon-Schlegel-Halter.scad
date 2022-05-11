difference() {
    difference() {
        union() {
            color("yellow", 1) roundedRect([44, 15, 3.5], 2);
            color("yellow", 1) translate([0, 0, 3.5]) rotate([90, 0, 0]) cylinder(17, 7.5, 7.5, true);
        }
        union() {
            color("red", 1)  translate([0, 0, -2]) cube([46, 19, 4], true);
            color("red", 1) translate([15, 5, 4]) rotate([0, 0, 90]) cylinder(25, 1.5, 1.5, true);
            color("red", 1) translate([15, -5, 4]) rotate([0, 0, 90]) cylinder(25, 1.5, 1.5, true);
            color("red", 1) translate([-15, 5, 4]) rotate([0, 0, 90]) cylinder(25, 1.5, 1.5, true);
            color("red", 1) translate([-15, -5, 4]) rotate([0, 0, 90]) cylinder(25, 1.5, 1.5, true);
        }
    }
    union() {
        color("blue", 1) translate([0, 0, 3.5]) rotate([90, 0, 0]) cylinder(25, 4, 4, true);
        color("blue", 1)  translate([0, 0, -1]) cube([8, 25, 9], true);
    }
}


module roundedRect(size, radius)
{
    x = size[0];
    y = size[1];
    z = size[2];

    linear_extrude(height=z)
    hull()
    {
        // place 4 circles in the corners, with the given radius
        translate([(-x/2)+(radius/2), (-y/2)+(radius/2), 0])
        circle(r=radius);

        translate([(x/2)-(radius/2), (-y/2)+(radius/2), 0])
        circle(r=radius);

        translate([(-x/2)+(radius/2), (y/2)-(radius/2), 0])
        circle(r=radius);

        translate([(x/2)-(radius/2), (y/2)-(radius/2), 0])
        circle(r=radius);
    }
}

