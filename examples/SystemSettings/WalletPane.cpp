#include "Settings.h"

namespace Examples::Settings {
    using namespace Cupertino;

    struct WalletModel {
        int shippingAddress = 0;
        int email = 0;
        int phone = 0;
        bool compatibleCards = false;
        bool orders = true;
    };

    // Wallet & Apple Pay as 512 Pixels captured it: no cards yet, the details Apple Pay fills in, and what Wallet may
    // collect.
    void WalletPane() {
        static WalletModel model;
        Form([] {
            // The empty card list: its row stands 8 pt further in than a form row, the label and the button centered.
            Section({.header = "Payment Cards"}, [] {
                Padding(EdgeInsets{0.0f, 8.0f, 0.0f, 0.0f}, [] {
                    HStack([] {
                        Text("Debit or Credit Card");
                        Spacer();
                        Button("Add Card\xE2\x80\xA6");
                    });
                });
            });
            Section({.header = "Payment Details"}, [] {
                Label("Transaction defaults", {.description = "Choose the default payment information you want to use when making a purchase with Apple Pay. Addresses and payment options can be changed at the time of transaction."});
                Picker("Shipping Address", &model.shippingAddress, {"United States"});
                Picker("Email", &model.email, {""});
                Picker("Phone", &model.phone, {""});
            });
            // Apple joins product names with a no-break space: "Apple Pay" wraps as one word.
            Section([] { Toggle("Compatible Cards", &model.compatibleCards, {.description = "Verifies that your saved cards in Safari AutoFill are compatible with Apple\xC2\xA0Pay and allows you to use them in Wallet."}); });
            Section([] { Toggle("Add Orders to Wallet", &model.orders, {.description = "Orders from participating merchants will be automatically added to Wallet on your iPhone."}); });
            // The help button stands 8.5 pt after the button here (@2x).
            TrailingButtons({.top = 10.0f, .spacing = 8.5f}, [] {
                Button("See how your data is managed\xE2\x80\xA6");
                HelpButton();
            });
        });
    }
} // namespace Examples::Settings
